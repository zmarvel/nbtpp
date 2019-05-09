
DATS = \
       end_tag.dat \
       byte_tag.dat \
       short_tag.dat \
       int_tag.dat \
       long_tag.dat \
       float_tag.dat \
       double_tag.dat \
       byte_array_tag.dat \
       int_array_tag.dat \
       long_array_tag.dat \
       string_tag.dat \
       list_byte_tag.dat \
       list_string_tag.dat \
       list_compound_tag.dat \
       compound_tag.dat \

HEXS = $(DATS:.dat=.hex)

check: $(DATS)

%.dat: %.hex
	xxd -r -p $< $@

.PHONY: clean
clean:
	rm -f $(DATS)
