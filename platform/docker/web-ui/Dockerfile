FROM node:20-alpine AS build

WORKDIR /app/apps/web-ui

COPY apps/web-ui/package.json apps/web-ui/package-lock.json ./

RUN npm ci

COPY apps/web-ui/ ./

RUN npm run build

FROM nginx:1.27-alpine

COPY platform/docker/web-ui/default.conf.template /etc/nginx/templates/default.conf.template
COPY platform/docker/web-ui/env.js.template /usr/share/nginx/html/env.js.template
COPY platform/docker/web-ui/40-env-js.sh /docker-entrypoint.d/40-env-js.sh
COPY --from=build /app/apps/web-ui/dist/ /usr/share/nginx/html/

RUN chmod +x /docker-entrypoint.d/40-env-js.sh
