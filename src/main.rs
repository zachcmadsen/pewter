use fled::Logger;
use log::{error, warn, LevelFilter};

static LOGGER: Logger = Logger;

const USAGE: &str = "\
Usage: fled [OPTIONS] [FILE]

Options:
    -h, --help       Print help info and exit
    -v, --version    Print version info and exit";

struct Args {
    help: bool,
    version: bool,
    filename: Option<String>,
}

fn parse_args() -> Args {
    let mut args = Args { help: false, version: false, filename: None };

    let mut pargs = pico_args::Arguments::from_env();

    if pargs.contains(["-h", "--help"]) {
        args.help = true;
        return args;
    }

    if pargs.contains(["-v", "--version"]) {
        args.version = true;
        return args;
    }

    args.filename = match pargs.opt_free_from_str() {
        Ok(Some(filename)) => Some(filename),
        _ => None,
    };

    let remaining = pargs.finish();
    if !remaining.is_empty() {
        let mut remaining_str = String::new();
        if let Some((last, remaining)) = remaining.split_last() {
            for arg in remaining {
                remaining_str.push_str(&arg.to_string_lossy());
                remaining_str.push(' ');
            }
            remaining_str.push_str(&last.to_string_lossy());
        }

        warn!("ignoring extra arguments: {}", remaining_str);
    }

    args
}

fn main() {
    if log::set_logger(&LOGGER).is_err() {
        eprintln!("error: could not initialize logger");
        std::process::exit(1);
    }
    log::set_max_level(LevelFilter::Debug);

    let args = parse_args();

    if args.help {
        println!("{}", USAGE);
        return;
    }

    if args.version {
        println!(
            "fled {} ({})",
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
