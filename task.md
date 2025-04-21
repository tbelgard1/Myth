# Myth II Docker Server & Client Connection Checklist

## 1. Update Server Code for Docker Compatibility
- [ ] Edit `spaghetti/utils/environment.h`:
  - [ ] Set `#define USERD_HOST "0.0.0.0"` so the server listens on all interfaces
- [ ] Edit `spaghetti/users_new/main.c`:
  - [ ] Set `LOCAL_AREA_NETWORK` to your Docker host's subnet in hex (e.g., `0xC0A80000` for `192.168.0.0`)

## 2. Rebuild Docker Images
- [ ] Run `docker-compose build` in the `mythdevserver` directory

## 3. Start Server Containers
- [ ] Run `docker-compose up` to start all Myth server services
- [ ] Use `docker ps` to verify containers are running
- [ ] Use `docker logs <container>` to check for errors

## 4. Verify Network and Ports
- [ ] Ensure ports (`3453`, `6226`, etc.) are mapped from container to host
- [ ] Use `netstat -an` inside the container to check listening on `0.0.0.0`
- [ ] Confirm Windows Firewall allows inbound connections on these ports

## 5. Configure Myth II Client
- [ ] Find your Windows host's IP address using `ipconfig`
- [ ] In the Myth II client, set the server address to your host's IP and the correct port (e.g., `3453`)
- [ ] Ensure the client and server are on the same subnet, or set up port forwarding for remote access

## 6. Test Connection
- [ ] Start the Myth II client and attempt to connect to the Dockerized server
- [ ] Check server and client logs for successful connection or errors

## 7. Troubleshooting
- [ ] If connection fails, re-check all above steps
- [ ] Inspect Docker, server, and firewall logs for issues
- [ ] Ensure no typos in IPs, ports, or config files

---

**Tip:** For most home setups, your subnet will be `192.168.x.0` or `10.0.x.0`. Convert this to hex and set `LOCAL_AREA_NETWORK` accordingly in `main.c`.

If you need help with any step, let me know!
