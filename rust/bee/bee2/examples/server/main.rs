// -- main.rs --

mod server;

use bee2::scropedlogger::ScropedLogger;

// --

fn main() -> Result<(), Box<dyn std::error::Error>> {
    #[allow(unused)]
    let logger = ScropedLogger::new();

    let s = server::Server;
    bee2::app::thread_pool_server(s)
}
