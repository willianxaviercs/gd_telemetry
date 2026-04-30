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
	@printf "  %-15s %s\n" "help"          "Shows this help"
	@printf "  %-15s %s\n" "build-all"     "Build entire project"
	@printf "  %-15s %s\n" "build-cpp"     "Build all cpp apps"
	@printf "  %-15s %s\n" "build-docker"  "Build all docker images"
	@printf "  %-15s %s\n" "run"           "Runs development environment"
	@printf "  %-15s %s\n" "stop"          "Stops development environment"

build-cpp:
	cmake -B $(BUILD_PATH) \
  		-DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake \
  		-DVCPKG_TARGET_TRIPLET=x64-linux
	cmake --build $(BUILD_PATH)

build-web-ui:
	docker build -t $(PROJECT_NAME)/web-ui:latest \
		-f platform/docker/web-ui/Dockerfile .

build-web-api:
	docker build -t $(PROJECT_NAME)/web-api:latest \
		-f platform/docker/web-api/Dockerfile .

build-docker-consumer:
	docker build -t $(PROJECT_NAME)/stream-consumer:latest \
		-f platform/docker/stream-consumer/Dockerfile .

build-docker-device:
	docker build -t $(PROJECT_NAME)/device-sim:latest \
		-f platform/docker/edge-device/Dockerfile .

build-docker-postgres:
	docker build -t $(PROJECT_NAME)/postgres:latest \
		-f platform/docker/postgres/Dockerfile .

build-docker-all: build-docker-consumer build-docker-device build-docker-postgres build-web-api build-web-ui

build-all: build-cpp build-docker

run: check-env
	$(COMPOSE) up -d --wait redis postgres stream-consumer web-api web-ui
	$(RUNTIME_BIN)/run.sh

check-env:
	@test -f .env || (echo ".env file missing - aborting build" && exit 1)

stop:
	$(RUNTIME_BIN)/stop.sh
	$(COMPOSE) down -v

