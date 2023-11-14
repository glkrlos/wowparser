mod staticdata;

fn print_header()
{
    println!("WoWParser Version {} ({}) in Pre-Alpha Development State for {} {} (Revision: {})", staticdata::version(), staticdata::codename(), staticdata::os(), staticdata::arch(), staticdata::revision());
    println!("Hash: {}\tDate: {}", staticdata::hash(), staticdata::date());
    println!("");
    println!("Tool to Parse World of Warcraft files (DBC DB2 ADB WDB).");
    println!("Copyright(c) 2023 Carlos Ramzuel - Huamantla, Tlaxcala, Mexico.");
    println!("");
}

fn pass1_loadconfig()
{
}

fn pass2_printfilestolog()
{
}

fn pass3_checkheadersanddataconsistency()
{
}

fn print_end()
{
    println!("-----> Finished");
}

fn main()
{
    print_header();

    pass1_loadconfig();

    pass2_printfilestolog();

    pass3_checkheadersanddataconsistency();

    print_end();
}
