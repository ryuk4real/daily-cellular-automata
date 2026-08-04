.PHONY: build daily test test_circular video clean

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

# Test mode - Circular Neighborhood (NC)
test_circular: build
	@echo "> Testing with Circular Neighborhood (NC)"
	@./cellular-automata/cellular_automata --test-circular
	@echo ""
	@python3 visualization/render_video.py output test_circular.mp4 30

clean:
	cd cellular-automata && $(MAKE) clean
	rm -rf output output_circular *.mp4
