use fled::Logger;
use log::{error, LevelFilter};

static LOGGER: Logger = Logger;

struct Args {
    help: bool,
    version: bool,
    filename: Option<String>,
}

fn parse_args() -> Result<Args, lexopt::Error> {
    use lexopt::prelude::*;

    let mut args = Args { help: false, version: false, filename: None };

    let mut parser = lexopt::Parser::from_env();
    while let Some(arg) = parser.next()? {
        match arg {
            Short('h') | Long("help") => {
                args.help = true;
            }
            Short('v') | Long("version") => {
                args.version = true;
            }
            Value(val) if args.filename.is_none() => {
                // TODO: Pretty sure I could handle this better.
                args.filename = Some(val.to_string_lossy().into_owned());
            }
            _ => return Err(arg.unexpected()),
        }
    }

    Ok(args)
}

fn usage() {
    println!(
        "\
Usage: {} [OPTIONS] [FILE]

Options:
    -h, --help       Print help info and exit
    -v, --version    Print version info and exit",
        env!("CARGO_PKG_NAME")
    );
}

fn main() {
    if log::set_logger(&LOGGER).is_err() {
        eprintln!("error: could not initialize logger");
        std::process::exit(1);
    }
    log::set_max_level(LevelFilter::Debug);

    let args = match parse_args() {
        Ok(args) => args,
        Err(err) => {
            error!("{}", err);
            std::process::exit(1);
        }
    };

    if args.help {
        usage();
        return;
    }

    if args.version {
        println!(
            "{} {} ({})",
            env!("CARGO_PKG_NAME"),
            env!("CARGO_PKG_VERSION"),
            env!("COMMIT_HASH")
        );
        return;
    }

    if let Err(x) = fled::run() {
        // TODO: Double check that this prints the context from anyhow errors.
        error!("{}", x.to_string());
        std::process::exit(1);
    }
}
