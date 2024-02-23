build:
	make -C getline_noblock
	make -C uci/locale
	make -C uci

clean:
	make clean -C getline_noblock
	make clean -C uci/locale
	make clean -C uci
