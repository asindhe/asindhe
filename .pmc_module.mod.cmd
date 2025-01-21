cmd_/root/akash/pmc_module.mod := printf '%s\n'   pmc_module.o | awk '!x[$$0]++ { print("/root/akash/"$$0) }' > /root/akash/pmc_module.mod
