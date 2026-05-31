.PHONY: build run test clean

build:
	cmake -S . -B build
	cmake --build build

run:
	@if [ -f build/Debug/lightkv_server.exe ]; then \
		build/Debug/lightkv_server.exe; \
	elif [ -f build/Release/lightkv_server.exe ]; then \
		build/Release/lightkv_server.exe; \
	elif [ -f build/lightkv_server.exe ]; then \
		build/lightkv_server.exe; \
	elif [ -f build/lightkv_server ]; then \
		./build/lightkv_server; \
	elif [ -f build/Debug/lightkv_server ]; then \
		./build/Debug/lightkv_server; \
	else \
		echo "lightkv_server not found. Run 'make build' first."; \
		exit 1; \
	fi

test:
	ctest --test-dir build --output-on-failure

clean:
	cmake -E remove_directory build

