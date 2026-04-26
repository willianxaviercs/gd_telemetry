FROM postgres:16-alpine

# Copy ALL schemas into init directory
COPY libs/schemas/postgres/ /docker-entrypoint-initdb.d/
