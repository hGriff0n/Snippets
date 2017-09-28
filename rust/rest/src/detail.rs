
extern crate chashmap;
extern crate syncbox;
extern crate serde;

use self::chashmap::CHashMap;
use self::syncbox::LinkedQueue;
use std::sync::Arc;

use self::serde::ser::*;
use self::serde::de::*;
use core::ops::Deref;
use std::fmt;

#[derive(Serialize, Deserialize)]
pub struct Server {
    dict: ServerDict,
    acts: ServerQueue,
}

impl Server {
    pub fn new() -> Server {
        Self{
            dict: ServerDict{ elem: Arc::new(CHashMap::new()) },
            acts: ServerQueue{ elem: LinkedQueue::new() },
        }
    }

    // These two methods simplify the accessing of the backing data structures 
    #[allow(non_snake_case)]
    pub fn getCache(&self) -> Arc<CHashMap<String, String>> {
        self.dict.elem.clone()
    }

    #[allow(non_snake_case)]
    pub fn getQueue(&self) -> LinkedQueue<Action> {
        self.acts.elem.clone()
    }
}

#[derive(Serialize, Deserialize)]
pub enum Action {
    Set(String, String),
    Delete(String)
}

/*
 * Wrapping structs for the concurrent data structures to enable serde integration
 *
 * Due to the "orphan rule" it is not possible to define a trait for an object outside
 *   of the module in which it (or the trait) was declared. This means that the
 *   concurrent structures don't have serde implementations. It's possible to "trick"
 *   rust into creating these definitions, but due to the presence of private data\
 *   fields, and since we probably shouldn't maintain the locks across server
 *   instances, these wrappers are used to focus specialize on what's necessary
 */
struct ServerDict {
    pub elem: Arc<CHashMap<String, String>>
}

impl Serialize for ServerDict {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let mut map = serializer.serialize_map(Some(self.elem.len()))?;
        for (k, v) in self.elem.deref().clone() {
            map.serialize_entry(&k, &v)?;
        }
        map.end()
    }
}

impl<'de> Deserialize<'de> for ServerDict {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct _Visitor;

        impl<'de> Visitor<'de> for _Visitor {
            type Value = ServerDict;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("hashmap")
            }

            fn visit_map<M: MapAccess<'de>>(self, mut access: M) -> Result<Self::Value, M::Error> {
                let dict = ServerDict{ elem: Arc::new(CHashMap::new()) };

                while let Some((key, value)) = access.next_entry()? {
                    dict.elem.insert(key, value);
                }

                Ok(dict)
            }
        }

        deserializer.deserialize_map(_Visitor)
    }
}

struct ServerQueue {
    pub elem: LinkedQueue<Action>
}

impl Serialize for ServerQueue {
    fn serialize<S: Serializer>(&self, serializer: S) -> Result<S::Ok, S::Error> {
        let mut seq = serializer.serialize_seq(Some(self.elem.len()))?;

        while !self.elem.is_empty() {
            seq.serialize_element(&self.elem.take())?;
        }
        
        seq.end()
    }
}

impl<'de> Deserialize<'de> for ServerQueue {
    fn deserialize<D: Deserializer<'de>>(deserializer: D) -> Result<Self, D::Error> {
        struct _Visitor;

        impl<'de> Visitor<'de> for _Visitor {
            type Value = ServerQueue;

            fn expecting(&self, formatter: &mut fmt::Formatter) -> fmt::Result {
                formatter.write_str("queue")
            }

            fn visit_seq<A: SeqAccess<'de>>(self, mut seq: A) -> Result<Self::Value, A::Error> {
                let queue = ServerQueue{ elem: LinkedQueue::new() };

                while let Some(value) = seq.next_element()? {
                    queue.elem.put(value);
                }

                Ok(queue)
            }
        }

        deserializer.deserialize_seq(_Visitor)
    }
}