DOREV := $(shell ./staticdata.sh)
all:
	rustc src/main.rs -o bin/wowparser3_AMD64
	rm src/staticdata.rs
