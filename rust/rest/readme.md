# REST Database

A simple HTTP key-value transactional database that responds to a group of 4 REST API endpoints (as defined in 'interview-homework.pdf')

## Basic Structure

The structure of this server is primarily defined through the 4 rest endpoints. Outside of these endpoints, the program is a simple usage of common concurrent data structures. Using serde, it is possible for the server state to be saved and loaded from a data file if one exists, enabling these structures to persist across server restarts. The file 'detail.rs' exists to structure the implementations of the serde implementations due rust orphanage rules. Lastly, an interrupt handler is registered for the 'Ctrl+c' keyboard shortcut to run the serialization right before server shutdown.

A single 'GET' endpoint is provided taking a single url "extension". Using nickel's macro/regex system, this extension is extracted from the url path and is then passed on to the key-value map. Rest is as stated.

The 'POST' set endpoint is slightly trickier as it first extracts a map from the provided json body. If a valid json dictionary isn't used, then the endpoint immediately returns a '404'. Otherwise, the endpoint ensures that only one entry was provided, returning the '400' signal otherwise. If the code passes these checks, then the 'key-value' pair is finally extracted from the map and a 'Set' action is pushed to the action queue. The return code is of course set according to whether the key already exists in the map.

The 'POST' commit endpoint is a simple loop over all elements within the action queue. For every action, the code switches on either inserting the 'key-value' or deleting the 'key' from the map.

The 'DELETE' endpoint maintains similar json parsing as with the 'POST/set' endpoint, with the difference of expecting a string instead. If a string is given, a 'Delete' action is pushed to the action queue.

## Included Libraries

    nickel: Rust http/rest server framework
        Chosen for the simplicity and robustness of creating the basic "REST" endpoints
    chashmap: Concurrent Hashmap
        nickel allocates a different thread for REST endpoint. Combined with rust's semantic
        rules, this necessitates the use of a concurrent data structure to maintain the backing
        database across the endpoints. Rust's standard library doesn't include such a structure
    syncbox: Concurrent Queue
        Same reasoning as chashmap but for a queue used to store the pre-commmit actions. There
        are better queues available, but none through cargo
    serde: Rust serialize/deserialize framework
        Structures the process for storing/loading the server map and queue from files
    serde_derive: Automatic derivation of serde implementations
    ctrlc: Ctrl-C registration library
        Simplifies registration of interrupt handler to save server state on shutdown
    bincode: Data formatting library
        Actually performs the reading/writing of server backing structures to files. bincode is a
        fairly compact binary representation compared to other libraries

### Improvements

    Switch out syncbox for the queue at https://github.com/kinghajj/deque/blob/master/src/lib.rs
    Add in a timer thread for saving the server state to the backup file
    Add in various logging capabilities/systems
    Add docopt arguments to enable specifying the port for server running
    Various improvements/refinements to the api/system specification

#### Setup and Operation

Cargo and Rust must be installed with a rust version of at least 1.17 (crates may require a higher version however).

To startup the server, simply run 'cargo run' or the included 'run.sh' script. An explicit build step is not required (run performs a build if necessary), but the 'build.sh' script is provided in case the existing system assumes this behavior.

After starting up the scripts, a rest server will be created at "127.0.0.1:4000". This server will continue running until it intercepts a 'ctrl+c' signal in which case it will save it's running state to the 'grayson_server.dat' file and exit (Not sure about this behavior otherwise).