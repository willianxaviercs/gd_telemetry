SHELL := /bin/bash
.DEFAULT_GOAL := help

COMPOSE := docker compose -f infra/docker-compose.yml
KEEP_DATA ?= 0

.PHONY: help collector-build infra-up infra-down infra-down-keep runtime-prepare runtime-start runtime-stop runtime-clean up down down-keep clean

help:
	@printf "Gundam local development\n\n"
	@printf "  make up             Build collector, start Redis/Postgres, prepare runtime, start simulators/collectors\n"
	@printf "  make down           Stop runtime, clean runtime state, remove Redis/Postgres data\n"
	@printf "  make down-keep      Stop runtime, clean runtime state, keep Redis/Postgres volumes\n"
	@printf "  make collector-build Build the collector binary\n"
	@printf "  make runtime-prepare Recreate per-device SQLite state under runtime/devices\n"
	@printf "  make runtime-start   Start simulators and collectors in background\n"
	@printf "  make runtime-stop    Stop background simulators and collectors\n"
	@printf "  make runtime-clean   Remove generated runtime state under runtime/\n"
	@printf "  make infra-up        Start Redis and Postgres only\n"
	@printf "  make infra-down      Stop Redis and Postgres, remove volumes by default\n"
	@printf "\n"
	@printf "Suggested flow:\n"
	@printf "  1. make up\n"
	@printf "  2. inspect logs under runtime/devices/<id>/\n"
	@printf "  3. make down    or    make down-keep\n"

collector-build:
	cd collector && ./build.sh

infra-up:
	$(COMPOSE) up -d --wait redis postgres

infra-down:
ifeq ($(KEEP_DATA),1)
	$(COMPOSE) down
else
	$(COMPOSE) down -v
endif

infra-down-keep:
	$(COMPOSE) down

runtime-prepare:
	./runtime/bin/prepare.sh

runtime-start:
	./runtime/bin/start.sh

runtime-stop:
	./runtime/bin/stop.sh

runtime-clean:
	./runtime/bin/clean.sh

up: collector-build infra-up runtime-prepare runtime-start

down: runtime-stop runtime-clean infra-down

down-keep:
	$(MAKE) down KEEP_DATA=1

clean: down
