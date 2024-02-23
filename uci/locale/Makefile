headers=locale/commands.h locale/responses.h
build: $(headers)
$(headers): generate_locale.py commands.txt responses.txt
	python3 generate_locale.py
clean:
	$(RM) $(headers)
