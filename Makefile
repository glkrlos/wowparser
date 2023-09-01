all: linux_build #windows_build

pre-build:
	@bash staticdata.sh

linux_build: pre-build
	@echo "Compiling for Linux"
	@cargo build --target=x86_64-unknown-linux-gnu
	@cp target/x86_64-unknown-linux-gnu/debug/wowparser4 bin/wowparser4_amd64
	@./bin/wowparser4_amd64

windows_build: pre-build
	@echo "Compiling for Windows"
	@cargo build --target=x86_64-pc-windows-gnu
	@cp target/x86_64-pc-windows-gnu/debug/wowparser4.exe bin/wowparser4_x64.exe

clean:
	@rm -rf target/
	@rm src/staticdata.rs
