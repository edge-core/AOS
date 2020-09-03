# Build the Docker image for people to use as a build image
# Only maintainers need this ... everyone else should use the 
# top level Makefile

IMAGE=9.1
ifndef PROJECT_NAME
$(error Environment variable PROJECT_NAME is undefined!)
endif

DOCKER=docker

all: docker_build docker_run

build: docker_build

run:
	export MAKEPID=$(shell echo $$PPID); \
	./run_docker.sh

delete: docker_delete

docker_delete: docker_check
	$(eval MAKEGID := $(shell ps -o pgid= $$PPID | grep -o [0-9]*))
	$(eval MAKEPID := $(if $(MAKEPID),$(MAKEPID),$(MAKEGID)))

	$(DOCKER) rm $(USER)-$(MAKEPID)

docker_build: docker_check
	$(DOCKER) build -t accton/build-$(PROJECT_NAME):$(IMAGE) .

docker_run: docker_check
	$(eval MAKEGID := $(shell ps -o pgid= $$PPID | grep -o [0-9]*))
	$(eval MAKEPID := $(if $(MAKEPID),$(MAKEPID),$(MAKEGID)))
	$(eval GITHASH := $(shell git describe --all --long --dirty || date -u +'%Y%m%d-%H%M%S UTC'))
	$(eval GITTOPD := $(shell git rev-parse --show-toplevel))
	$(eval GITKHAS := $(shell git --git-dir=$(GITTOPD)/.git rev-list --abbrev-commit -1 HEAD -- **/linux-4.19.67/arch/x86/configs/aos5810_54x_defconfig))

	@echo GITHASH=$(GITHASH) > ../githash.txt
	@echo GITKHAS=$(GITKHAS) >> ../githash.txt
	@echo GITHASH=$(GITHASH)
	@echo GITKHAS=$(GITKHAS)
	@echo SHARE_FOLDER=$(shell cd ../../../ && pwd) 
	@echo $(DOCKER) run -t -i -v $$SHARE_FOLDER:/home/$(PROJECT_NAME)
	@echo --name=$(USER)-$(MAKEPID)
	@echo -w="/home/$(PROJECT_NAME)/project_build_environment/$(PROJECT_NAME)/utils"
	@echo accton/build-$(PROJECT_NAME):$(IMAGE)
	@export SHARE_FOLDER=`cd ../../../ && pwd` && \
	$(DOCKER) run -t -i -v $$SHARE_FOLDER:/home/$(PROJECT_NAME):Z \
	--name=$(USER)-$(MAKEPID) --privileged \
	-w="/home/$(PROJECT_NAME)/project_build_environment/$(PROJECT_NAME)/utils" \
	accton/build-$(PROJECT_NAME):$(IMAGE) bash --rcfile ./build_env_init

        

# DANGER - destructively kills all containers and images - DO NOT USE
#docker_nuke_all: docker_check
#	@echo Before
#	@echo ------------------------
#	$(DOCKER) info
#	$(DOCKER) rm `docker ps -aq` || true
#	$(DOCKER) rmi `docker images -aq` || true
#	@echo After
#	@echo ------------------------
#	$(DOCKER) info

#docker_push: docker_check
#	$(DOCKER) push accton/build-$(PROJECT_NAME)

docker_check: 
	@which $(DOCKER) || echo STOP : you need to install docker.io first
