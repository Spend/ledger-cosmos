#*******************************************************************************
#*   (c) 2019 ZondaX GmbH
#*
#*  Licensed under the Apache License, Version 2.0 (the "License");
#*  you may not use this file except in compliance with the License.
#*  You may obtain a copy of the License at
#*
#*      http://www.apache.org/licenses/LICENSE-2.0
#*
#*  Unless required by applicable law or agreed to in writing, software
#*  distributed under the License is distributed on an "AS IS" BASIS,
#*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#*  See the License for the specific language governing permissions and
#*  limitations under the License.
#********************************************************************************

.PHONY: all deps build clean load delete

LEDGER_COSMOS_SRC=$(CURDIR)/src/ledger-user
LEDGER_TENDERMINT_SRC=$(CURDIR)/src/ledger-val

DOCKER_IMAGE=zondax/ledger-docker-bolos:latest
DOCKER_BOLOS_SDK=/project/deps/nanos-secure-sdk
DOCKER_BOLOS_SDKX=/project/deps/nano2-sdk

SCP_PUBKEY=049bc79d139c70c83a4b19e8922e5ee3e0080bb14a2e8b0752aa42cda90a1463f689b0fa68c1c0246845c2074787b649d0d8a6c0b97d4607065eee3057bdf16b83
SCP_PRIVKEY=ff701d781f43ce106f72dc26a46b6a83e053b5d07bb3d4ceab79c91ca822a66b

all: build

define run_docker_privileged
	docker run -i --rm \
	--privileged \
	-e SCP_PRIVKEY=$(SCP_PRIVKEY) \
	-e BOLOS_SDK=$(1) -e BOLOS_ENV=/opt/bolos \
	-u $(shell id -u) \
	-v $(shell pwd):/project \
	$(DOCKER_IMAGE) \
	$(2)
endef

deps:
	@echo "Install dependencies"
	$(CURDIR)/src/install_deps.sh

build_cosmos:
	@cp $(LEDGER_COSMOS_SRC)/nanos_icon.gif $(LEDGER_COSMOS_SRC)/glyphs/icon_app.gif
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-user)

build_cosmosX:
	@cp $(LEDGER_COSMOS_SRC)/nanos_icon.gif $(LEDGER_COSMOS_SRC)/glyphs/icon_app.gif
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDKX),make -C /project/src/ledger-user)

build_tendermint:
	@cp $(LEDGER_TENDERMINT_SRC)/nanos_icon.gif $(LEDGER_TENDERMINT_SRC)/glyphs/icon_app.gif
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-val)

build_tendermintX:
	@cp $(LEDGER_TENDERMINT_SRC)/nanos_icon.gif $(LEDGER_TENDERMINT_SRC)/glyphs/icon_app.gif
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDKX),make -C /project/src/ledger-val)

clean_cosmos:
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-user clean)

clean_tendermint:
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-val clean)

load_cosmos: build_cosmos
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-user load)

load_cosmosX: build_cosmosX
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDKX),make -C /project/src/ledger-user load)

load_tendermint: build_tendermint
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-val load)

load_tendermintX: build_tendermintX
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDKX),make -C /project/src/ledger-val load)

delete_cosmos:
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-user delete)

delete_tendermint:
	$(call run_docker_privileged,$(DOCKER_BOLOS_SDK),make -C /project/src/ledger-val delete)

# This target will initialize the device with the integration testing mnemonic
dev_init:
	@echo "Initializing device with test mnemonic! WARNING TAKES 2 MINUTES AND REQUIRES RECOVERY MODE"
	@python -m ledgerblue.hostOnboard --apdu --id 0 --prefix "" --passphrase "" --pin 5555 --words "equip will roof matter pink blind book anxiety banner elbow sun young"

# This target will setup a custom developer certificate
dev_ca:
	@python -m ledgerblue.setupCustomCA --targetId 0x31100004 --public $(SCP_PUBKEY) --name zondax

dev_ca_delete:
	@python -m ledgerblue.resetCustomCA --targetId 0x31100004

# This target will setup a custom developer certificate
dev_ca2:
	@python -m ledgerblue.setupCustomCA --targetId 0x33000004 --public $(SCP_PUBKEY) --name zondax

dev_ca_delete2:
	@python -m ledgerblue.resetCustomCA --targetId 0x33000004

clean: clean_cosmos clean_tendermint
build: build_cosmos build_tendermint
