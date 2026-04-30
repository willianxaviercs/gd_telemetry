FROM node:20-alpine

WORKDIR /app/apps/web-api

COPY apps/web-api/package.json apps/web-api/package-lock.json ./

RUN npm ci --omit=dev

COPY apps/web-api/src ./src
COPY libs /app/libs
