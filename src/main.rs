const USAGE: &str = "\
Usage: fled [OPTIONS] FILE

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

    // TODO: Add debug and warn logging.
    args.filename = match pargs.opt_free_from_str() {
        Ok(Some(filename)) => Some(filename),
        _ => None,
    };

    let remaining = pargs.finish();
    if !remaining.is_empty() {
        eprint!("warn: ignoring extra arguments: ");
        if let Some((last, remaining)) = remaining.split_last() {
            for arg in remaining {
                eprint!("{} ", arg.to_string_lossy());
            }
            eprint!("{}", last.to_string_lossy());
        }
    }

    args
}

fn main() {
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
        eprintln!("error: {}", x.to_string());
        std::process::exit(1);
    }
}
