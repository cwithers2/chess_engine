headers=locale/commands.h locale/responses.h
build: $(headers)
$(headers): generate_locale.py commands.txt responses.txt
	@mkdir -p ./locale
	@mkdir -p ./src
	python3 generate_locale.py
clean:
	$(RM) -R ./locale
	$(RM) -R ./src
