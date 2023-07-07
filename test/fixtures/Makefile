platform-%.tar:
	tar \
	--create \
	--exclude-from .gitignore \
	--file $@ \
	--directory platform/by-dkey/aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa/0 \
	by-arch/$*/ \
	corestore/

all: \
	platform-darwin-arm64.tar \
	platform-darwin-x64.tar \
	platform-linux-arm64.tar \
	platform-linux-x64.tar \
	platform-win32-x64.tar
