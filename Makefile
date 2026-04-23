.PHONY: help build run stop clean
.DEFAULT_GOAL := help

SHELL := /bin/bash
COMPOSE := docker compose -f platform/docker/docker-compose.yml
RUNTIME_BIN := ./platform/runtime/bin
BUILD_PATH := build

help:
	@printf "Project development workflow\n\n"
	@printf "commands:\n\n"
	@printf "  %-10s %s\n" "build" "Build entire project"
	@printf "  %-10s %s\n" "run"   "Run development environment"
	@printf "  %-10s %s\n" "stop"  "Stop development environment"

build:
	cmake -S . -B $(BUILD_PATH)
	cmake --build $(BUILD_PATH)

run: build
	$(COMPOSE) up -d --wait redis postgres
	$(RUNTIME_BIN)/prepare.sh
	$(RUNTIME_BIN)/start.sh

stop:
	$(RUNTIME_BIN)/stop.sh
	$(RUNTIME_BIN)/clean.sh
	$(COMPOSE) down -v

clean:

