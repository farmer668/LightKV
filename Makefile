.PHONY: all build run test cli bench clean

all: build

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
	elif [ -f build/Release/lightkv_server ]; then \
		./build/Release/lightkv_server; \
	else \
		echo "lightkv_server not found. Run 'make' first."; \
		exit 1; \
	fi

test:
	ctest --test-dir build --output-on-failure

cli:
	@if [ -f build/Debug/lightkv_cli.exe ]; then \
		build/Debug/lightkv_cli.exe; \
	elif [ -f build/Release/lightkv_cli.exe ]; then \
		build/Release/lightkv_cli.exe; \
	elif [ -f build/lightkv_cli.exe ]; then \
		build/lightkv_cli.exe; \
	elif [ -f build/lightkv_cli ]; then \
		./build/lightkv_cli; \
	elif [ -f build/Debug/lightkv_cli ]; then \
		./build/Debug/lightkv_cli; \
	elif [ -f build/Release/lightkv_cli ]; then \
		./build/Release/lightkv_cli; \
	else \
		echo "lightkv_cli not found. Run 'make' first."; \
		exit 1; \
	fi

bench:
	@if [ -f build/Debug/lightkv_bench.exe ]; then \
		build/Debug/lightkv_bench.exe --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	elif [ -f build/Release/lightkv_bench.exe ]; then \
		build/Release/lightkv_bench.exe --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	elif [ -f build/lightkv_bench.exe ]; then \
		build/lightkv_bench.exe --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	elif [ -f build/lightkv_bench ]; then \
		./build/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	elif [ -f build/Debug/lightkv_bench ]; then \
		./build/Debug/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	elif [ -f build/Release/lightkv_bench ]; then \
		./build/Release/lightkv_bench --host 127.0.0.1 --port 6379 --clients 10 --requests 1000 --read-ratio 0.8; \
	else \
		echo "lightkv_bench not found. Run 'make' first."; \
		exit 1; \
	fi

clean:
	cmake -E remove_directory build
