
// Module declarations
#[macro_use]
extern crate nickel;
extern crate core;
extern crate ctrlc;
#[macro_use]
extern crate serde_derive;
extern crate bincode;

mod detail;


// Imports
use nickel::*;
use std::collections::HashMap;
use core::ops::Deref;
use detail::Action;

// Import used status codes:    200, 404,      201,     400,        204
use nickel::status::StatusCode::{Ok, NotFound, Created, BadRequest, NoContent};

const DATA_FILE: &'static str = "grayson_server.dat";

fn main() {
    let mut api = Nickel::new();

    // Extract the server backing data, possibly from a database file
    let server = std::fs::File::open(DATA_FILE)
        .map(|mut file| bincode::deserialize_from(&mut file, bincode::Infinite).expect("corrupted database"))
        .unwrap_or_else(|_| detail::Server::new());
    

    // Create clones of the server structures to get around the closure rules
    let cache = server.getCache();
    let com_cache = server.getCache();
    let set_cache = server.getCache();
    let queue = server.getQueue();
    let set_queue = server.getQueue();
    let del_queue = server.getQueue();


    // Specify the api endpoints
    // NOTE: Anything not picked up by these handlers is a '404'
    api.utilize(router! {
        get "/:key" => |request, mut response| {
            let key = request.param("key").unwrap();

            match cache.get(&key.to_string()) {
                Some(val) => {
                    response.set(Ok);
                    format!("{{ \"{}\": \"{}\" }}", key, val.deref())
                },
                None => {
                    response.set(NotFound);
                    "".to_string()
                }
            }
        }

        post "/set" => |request, mut response| {
            match request.json_as::<HashMap<String, String>>() {
                Result::Ok(map) => {
                    if map.len() == 1 {
                        let key = map.keys().next().unwrap().clone();
                        let val = map.get(&key).unwrap().clone();
                        
                        match set_cache.contains_key(&key) {
                            true => { response.set(Ok); },
                            false => { response.set(Created); }
                        }

                        set_queue.put(Action::Set(key, val));

                    } else {
                        response.set(BadRequest);
                    }

                    ""
                },
                Result::Err(_) => {
                    response.set(NotFound);
                    "Invalid json body"
                },
            }
        }

        // You know, this isn't quite "stateless"
        post "/commit" => |_, mut response| {
            while !queue.is_empty() {
                match queue.take() {
                    Action::Set(key, val) => { com_cache.insert(key.clone(), val); },
                    Action::Delete(key) => { com_cache.remove(&key); }
                }
            }

            response.set(NoContent);
            ""
        }

        delete "/set" => |request, mut response| {
            match request.json_as::<String>() {
                Result::Ok(key) => {
                    response.set(Ok);
                    del_queue.put(Action::Delete(key));
                    ""
                },
                Result::Err(_) => {
                    response.set(NotFound);
                    "Invalid json body"
                },
            }
        }
    }); 

    // Register a handler to save server state on shutdown (not sure if this handles all cases)
    match api.listen("127.0.0.1:4000") {
        Result::Ok(_) =>
            ctrlc::set_handler(move || {
                let mut f = std::fs::File::create(DATA_FILE).expect("creating database");
                bincode::serialize_into(&mut f, &server, bincode::Infinite).expect("serializing");
            }).expect("ctrl-c handler"),
        Result::Err(err) => println!("Error: {:?}", err)
    }
}
