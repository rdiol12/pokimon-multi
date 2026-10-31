# Netplay server: game port (TCP) plus the leaderboard page and bridge
# download (HTTP). Build from the repository root:
#   docker build -f deploy/Dockerfile -t pokimon-netplay .
FROM python:3.12-slim

WORKDIR /app
COPY server/*.py server/
COPY tools/netplay_bridge/*.lua tools/netplay_bridge/

RUN useradd --system --home-dir /data netplay && mkdir -p /data && chown netplay /data
USER netplay
VOLUME /data

ENV NETPLAY_PORT=7777 \
    NETPLAY_HTTP_PORT=8080 \
    NETPLAY_DB=/data/league.sqlite3 \
    PYTHONUNBUFFERED=1
EXPOSE 7777 8080

HEALTHCHECK --interval=30s --timeout=3s \
    CMD python3 -c "import urllib.request; urllib.request.urlopen('http://127.0.0.1:8080/health', timeout=2)"

CMD ["python3", "server/netplay_server.py"]
