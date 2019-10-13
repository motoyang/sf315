// -- main.rs --

mod client;

use bee2::scropedlogger::ScropedLogger;

// --

fn main() {
    #[allow(unused)]
    let logger = ScropedLogger::new();

    let c = client::Client;
    bee2::app::single_thread_client(c);
}
