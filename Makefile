.PHONY: help build run stop clean
.DEFAULT_GOAL := help

SHELL := /bin/bash
COMPOSE := docker compose -f platform/docker/docker-compose.yml
RUNTIME_BIN := ./platform/runtime/tools
BUILD_PATH := build

help:
	@printf "Project development workflow\n\n"
	@printf "commands:\n\n"
	@printf "  %-10s %s\n" "build" "Build entire project"
	@printf "  %-10s %s\n" "run"   "Run development environment"
	@printf "  %-10s %s\n" "stop"  "Stop development environment"

build:
	cmake -B $(BUILD_PATH) \
  		-DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake \
  		-DVCPKG_TARGET_TRIPLET=x64-linux
	cmake --build $(BUILD_PATH)

	docker build -t device-sim -f platform/runtime/Dockerfile .

run: build
	$(COMPOSE) up -d --wait redis postgres
	$(RUNTIME_BIN)/run.sh

stop:
	$(RUNTIME_BIN)/stop.sh
	$(COMPOSE) down -v

