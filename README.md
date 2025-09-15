# handover-xapp

An O-RAN RIC xApp written in C++17 that triggers UE handovers by sending E2AP RIC Control messages (using E2SM RC Control Format 1) based on A1 policies.


## Overview

handover-xapp listens for A1 policy requests, validates/parses their payload, looks up target E2 nodes via E2MGR, builds an E2AP RIC Control Request with E2SM-RC content, and sends it over RMR to the selected E2 node. It acknowledges each policy to the A1 Mediator with an A1_POLICY_RESP (OK or ERROR).

Key points:
- Receives: A1_POLICY_REQ, RIC_CONTROL_ACK/FAILURE
- Sends: RIC_CONTROL_REQ, A1_POLICY_QUERY, A1_POLICY_RESP
- Policy Type used in examples: 5 (UE energy/connection management)


## Architecture

- Entry point: `src/main.cpp` creates the xApp (ricxfcpp), registers callbacks, and runs the RMR loop.
- A1 handling: `src/a1/a1mgr.*` validates and parses the policy JSON, yielding a list of target E2 nodes and UE IMSIs.
- E2 node discovery: `src/e2/e2mgr.*` queries E2MGR over HTTP to map PLMN + gNB ID to an inventory name (MEID) for directed RMR sends.
- E2 control build: `src/e2/control.*` builds E2SM RC Control Header/Message (Format 1) and wraps them in an E2AP RIC Control Request PDU. ASN.1 helpers are in `src/e2/utils.*`; generated ASN.1 modules live under `ASN1c/`.
- Messaging: RMR is used for A1 and E2 control messages. Initial routes are in `init/routes.txt` and augmented by RTMGR at runtime.
- Logging: `mdclog` with level set via `init/config-map.yaml`.


## Build

Container builds are the recommended path because native dependencies (RMR, mdclog, ricxfcpp, Pistache, RapidJSON, nlohmann JSON, json-schema-validator, ASN.1) are resolved in the multi-stage Dockerfile.

### Option A: Build container image

Prereqs: Docker

- Using the helper script (tags and optionally pushes):

```bash
./build_container_image.sh
```

The script builds image `handover-xapp:TNSM-25`, tags it as `zanattabruno/handover-xapp:TNSM-25`, and pushes if you are logged in to Docker Hub. Edit `build_container_image.sh` to change the tag or registry.

- Building directly via Dockerfile:

```bash
DOCKER_BUILDKIT=1 docker build -t handover-xapp:latest .
```

The Dockerfile compiles ric-plt/xapp-frame-cpp at a pinned commit (with a local patch applied), installs the native libs, builds this repo, and assembles a slim runtime image.

### Option B: Local build (advanced)

Local CMake builds are possible if you install all the native dependencies in compatible versions (see Dockerfile for a working bill of materials). Then:

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
sudo make install
```

Note: You may need ldconfig and library paths for RMR/mdclog/ricxfcpp depending on your environment.


## Run

At runtime the xApp needs an RMR routing table and a logging config. The container image already places defaults and sets envs:
- `CONFIG_MAP_NAME=/usr/local/etc/handover-xapp/config-map.yaml`
- `RMR_SEED_RT=/usr/local/etc/handover-xapp/routes.txt`

### Run the container

```bash
docker run --rm \
	--name handover-xapp \
	-e CONFIG_MAP_NAME=/usr/local/etc/handover-xapp/config-map.yaml \
	-e RMR_SEED_RT=/usr/local/etc/handover-xapp/routes.txt \
	handover-xapp:latest
```

In-cluster, the xApp registers with APPMGR and communicates with A1 Mediator, E2MGR, and E2TERM over RMR/HTTP according to your RIC deployment. See Helm below for Kubernetes.

### Run the binary (local install)

```bash
CONFIG_MAP_NAME=/path/to/config-map.yaml \
RMR_SEED_RT=/path/to/routes.txt \
handover-xapp
```


## Configuration

- RMR routes seed: `init/routes.txt`
	- Seeds routes for A1 Mediator and SubMgr; RTMGR will refine these dynamically.
- MDC log config: `init/config-map.yaml`
	- `log-level: ERR|WARN|INFO|DEBUG` (default: INFO)
- xApp descriptor (reference for Helm): `init/xapp-descriptor.json`
	- Defines ports, messages, and policy types (uses policy 5 in examples).
- Helm chart example: `debugger-xapp/` (uses the built image and descriptor values)

External services expected (cluster DNS shown for default namespaces):
- E2MGR HTTP: `http://service-ricplt-e2mgr-http.ricplt:3800`
- APPMGR HTTP: `http://service-ricplt-appmgr-http.ricplt:8080`


## A1 Policy schema and example

The xApp expects A1 policy requests with a payload describing E2 nodes and UE IMSIs. The code reads an A1 RMR message with fields:
- `operation`: string
- `policy_type_id`: string or number (e.g., 5)
- `policy_instance_id`: string
- `payload`: a JSON string containing the following object

Policy payload schema (from `init/E2nodeUESchema.json`, type 5):

```json
{
	"E2NodeList": [
		{
			"mcc": "<string>",
			"mnc": "<string>",
			"gnbid": "<string>",
			"pci": "<string>",
			"UEList": [ { "imsi": "<string>" } ]
		}
	]
}
```

Example policy payload instance (see `init/E2nodeUEInstance.json`):

```json
{
	"E2NodeList": [
		{
			"mcc": "310",
			"mnc": "260",
			"gnbid": "10001",
			"pci": "1",
			"UEList": [
				{ "imsi": "001001000000000" },
				{ "imsi": "001001000000001" }
			]
		},
		{
			"mcc": "310",
			"mnc": "260",
			"gnbid": "10001",
			"pci": "2",
			"UEList": [ { "imsi": "001001000000002" } ]
		}
	]
}
```

Note: The A1 RMR message’s `payload` field is a JSON string of the object above (stringified JSON), which the xApp parses internally.


## Message flow (high level)

1) Startup: xApp registers with APPMGR, seeds RMR routes, and can issue an A1_POLICY_QUERY for configured policy types.
2) A1_POLICY_REQ received: `A1Manager` parses `payload` into E2 targets and UE IMSIs.
3) E2 node lookup: `E2MgrCli` queries E2MGR for CONNECTED nodes, mapping PLMN + gNB ID (29-bit) to an inventory name (MEID).
4) Control build: For each UE, build E2SM-RC Control Header/Message (Fmt1), encode to E2AP RIC Control Request (RAN function ID currently hard-coded to 1).
5) Send: RIC_CONTROL_REQ is sent over RMR with MEID set to the target E2 node. ACK/FAIL is logged.
6) Respond: A1_POLICY_RESP is sent back to A1 Mediator with status OK/ERROR.


## Helm deployment (example)

An example Helm chart is provided under `debugger-xapp/`. It references the image `zanattabruno/handover-xapp:TNSM-25` by default (see `values.yaml`). To deploy with your own image:

```bash
# If you built and pushed your image to a registry, set it here
helm upgrade --install handover-xapp ./debugger-xapp \
	--set containers[0].image.registry=<your-registry> \
	--set containers[0].image.name=handover-xapp \
	--set containers[0].image.tag=<your-tag>
```

Ensure your RIC platform (ricplt namespace) and xApp namespace are running and that A1 Mediator, E2MGR, E2TERM, RTMGR, and APPMGR are healthy.


## Development notes

Repo layout:
- `src/main.cpp`: xApp bootstrap and callbacks
- `src/a1/a1mgr.*`: A1 message parsing and A1 response serialization
- `src/e2/control.*`: E2SM-RC Control builders and E2AP wrapping
- `src/e2/e2mgr.*`: HTTP client to E2MGR and node mapping helpers
- `src/e2/utils.*`: ASN.1 encode/decode helpers and NR-CGI/PLMN utilities
- `ASN1c/`: Generated E2AP/E2SM modules and asn1c runtime
- `init/`: RMR routes, logging config, xApp descriptor, and A1 schema/examples

Conventions:
- Logging via `mdclog_write(MDCLOG_*, ...)` (level from `config-map.yaml`).
- E2 node ID uses `GNB_ID_LENGTH = 29` bits; values above are rejected.
- RAN function ID is currently hard-coded to 1 in `main.cpp`.


## Troubleshooting

- No A1 messages received
	- Check `init/routes.txt` is mounted/used and RTMGR updated routes for A1 Mediator.
	- Verify A1 Mediator shows the xApp as registered and policy type 5 is configured.

- E2 node not found or mapping empty
	- Ensure E2MGR reports the E2 node as CONNECTED; check PLMN (hex) and gNB ID match platform inventory.
	- Confirm gNB ID is within 29 bits.

- RIC_CONTROL_REQ not acknowledged
	- Verify the target E2 node supports the RAN function ID 1 and E2SM-RC control style expected by this xApp.
	- Inspect E2TERM/E2 node logs for ASN.1 decode errors.

- Payload parse errors
	- Ensure the A1 message `payload` field is a JSON string of the schema shown above and that `UEList` is non-empty.

- Verbose logs
	- Set `log-level: DEBUG` in `init/config-map.yaml` and restart.


## License

Licensed under the Apache License, Version 2.0. See `LICENSE`.
