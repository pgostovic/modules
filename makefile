seed:
	make -f mk/seed.mk $(patsubst seed,,$(MAKECMDGOALS))

rack:
	arch -x86_64 make -f mk/rack.mk $(patsubst rack,,$(MAKECMDGOALS))

.DEFAULT:
	@echo $@

