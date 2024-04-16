use fled::Logger;
use log::{error, LevelFilter};

static LOGGER: Logger = Logger;

struct Args {
    help: bool,
    version: bool,
    filename: Option<String>,
}

fn parse_args() -> Option<Args> {
    let mut args = Args { help: false, version: false, filename: None };

    let mut pargs = pico_args::Arguments::from_env();

    if pargs.contains(["-h", "--help"]) {
        args.help = true;
        return Some(args);
    }

    if pargs.contains(["-v", "--version"]) {
        args.version = true;
        return Some(args);
    }

    args.filename = match pargs.opt_free_from_str() {
        Ok(Some(filename)) => Some(filename),
        _ => None,
    };

    let remaining = pargs.finish();
    if !remaining.is_empty() {
        return None;
    }

    Some(args)
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

    let Some(args) = parse_args() else {
        usage();
        std::process::exit(1);
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
