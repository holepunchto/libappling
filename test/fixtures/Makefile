platform-%.tar:
	tar \
	--create \
	--file $@ \
	--directory platform \
	stable/swap-0/checkout \
	stable/swap-0/bin/$*

all: \
	platform-darwin-arm64.tar \
	platform-darwin-x64.tar \
	platform-linux-arm64.tar \
	platform-linux-x64.tar \
	platform-win32-x64.tar
