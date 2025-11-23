.PHONY: build daily test video clean

build:
	cd cellular-automata && $(MAKE)

# Daily mode - random configuration based on date
daily: build
	@echo "> Daily Cellular Automata"
	@date
	@if [ -z "$(SEED)" ]; then \
		./cellular-automata/cellular_automata --daily; \
	else \
		./cellular-automata/cellular_automata --seed $(SEED); \
	fi
	@echo ""
	@python3 visualization/render_video.py output automata.mp4 duration=30
	@echo ""
	@date

# Test mode - Conway's Game of Life
test: build
	@echo "> Testing with Conway's Game of Life"
	@./cellular-automata/cellular_automata --test
	@echo ""
	@python3 visualization/render_video.py output test.mp4 30

clean:
	cd cellular-automata && $(MAKE) clean
	rm -rf output *.mp4
