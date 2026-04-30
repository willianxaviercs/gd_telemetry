#!/bin/sh
set -eu

envsubst '${API_HOST} ${API_PORT}' \
  < /usr/share/nginx/html/env.js.template \
  > /usr/share/nginx/html/env.js
