cmd_/root/akash/pmc_module.ko := ld -r -m elf_x86_64 -z noexecstack --build-id=sha1  -T scripts/module.lds -o /root/akash/pmc_module.ko /root/akash/pmc_module.o /root/akash/pmc_module.mod.o;  true
