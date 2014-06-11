all: coding-guidelines.pdf

%.pdf: %.html style.css
	wkhtmltopdf $< $@ \
		--margin-bottom 0.75in \
		--margin-top 0.75in \
		--margin-left 0.75in \
		--margin-right 0.75in

%.html: %.md
	markdown $< > $@

clean:
	rm -f *.pdf *.html

