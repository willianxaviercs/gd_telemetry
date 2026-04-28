.PHONY: help build run stop clean
.DEFAULT_GOAL := help

SHELL := /bin/bash
COMPOSE := docker compose --env-file .env -f platform/docker/backend/docker-compose.yml
RUNTIME_BIN := ./platform/runtime/tools
BUILD_PATH := build
PROJECT_NAME := telemetry

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

	docker build -t $(PROJECT_NAME)/postgres:latest \
		-f platform/docker/postgres/Dockerfile .

	docker build -t $(PROJECT_NAME)/device-sim:latest \
		-f platform/docker/edge-device/Dockerfile .

	docker build -t $(PROJECT_NAME)/stream-consumer:latest \
		-f platform/docker/stream-consumer/Dockerfile .

run: build check-env
	$(COMPOSE) up -d --wait redis postgres stream-consumer
	$(RUNTIME_BIN)/run.sh

check-env:
	@test -f .env || (echo ".env file missing - aborting build" && exit 1)

stop:
	$(RUNTIME_BIN)/stop.sh
	$(COMPOSE) down -v

