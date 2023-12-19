all: linux_build windows_build

pre-build:
	@bash staticdata.sh

linux_build: pre-build
	@echo "Compiling for Linux"
	@cargo build --target=x86_64-unknown-linux-gnu

windows_build: pre-build
	@echo "Compiling for Windows"
	@cargo build --target=x86_64-pc-windows-gnu

clean:
	@rm -rf target/
	@rm src/staticdata.rs
