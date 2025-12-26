## llm slop

DOMAIN = wampy
POT_FILE = $(DOMAIN).pot
SRC_FILES = $(wildcard ../src/*/*.cpp) $(wildcard ../src/*.cpp) $(wildcard ../src/*/*.h) $(wildcard ../src/*.h) 
PO_FILES_DIR = po
MO_FILES_DIR = locale
LOCALES_DIR = locales
MSGID_BUGS_ADDRESS = unknown.at.github@yandex.com

LANGUAGES = en ru ja zh_CN vi es fr de ko pt_BR it pl

# PO files for each language - EXPLICIT TARGETS
PO_FILES = $(addprefix $(PO_FILES_DIR)/,$(addsuffix .po,$(LANGUAGES)))

LOCALES = $(addprefix $(LOCALES_DIR)/,$(LANGUAGES))

# MO files structure for each language
MO_FILES = $(foreach lang,$(LANGUAGES),$(MO_FILES_DIR)/$(lang)/LC_MESSAGES/$(DOMAIN).mo)
RANGE_FILES = $(foreach lang,$(LANGUAGES),$(MO_FILES_DIR)/$(lang)/LC_MESSAGES/range)

# Default target
all: init-languages mo locales.tar.gz

# Extract strings from source files and create POT file
$(POT_FILE): $(SRC_FILES)
	@echo "Extracting strings from source files..."
	xgettext --keyword=_ --keyword=N_ --keyword=Q_ \
	         --keyword=C_:1c,2 --keyword=NC_:1c,2 \
	         --keyword=gettext --keyword=ngettext:1,2 \
	         --from-code=UTF-8 \
	         --add-comments=TRANSLATORS: \
	         --package-name="$(DOMAIN)" \
	         --package-version="1.0" \
	         --msgid-bugs-address="$(MSGID_BUGS_ADDRESS)" \
	         -o $(POT_FILE) $(SRC_FILES)
	echo $(SRC_FILES)
	@echo "POT file created: $(POT_FILE)"

# Create PO files directory
$(PO_FILES_DIR):
	@mkdir -p $(PO_FILES_DIR)

# Initialize PO files for each language - PO_FILES are explicit targets
init-languages: $(POT_FILE) $(PO_FILES_DIR) $(PO_FILES)
	@echo "PO files initialized for languages: $(LANGUAGES)"

$(PO_FILES): $(POT_FILE)
	@lang=$(patsubst $(PO_FILES_DIR)/%.po,%,$@); \
	echo "Initializing/updating $$lang language..."; \
	if [ -f "$@" ]; then \
		echo "Updating existing PO file for $$lang..."; \
		msgmerge --update --backup=none "$@" $(POT_FILE); \
	else \
		msginit --no-translator --locale=$$lang --input=$(POT_FILE) --output="$@"; \
	fi;

locales.tar.gz:
	./locales.sh

$(MO_FILES_DIR)/%/LC_MESSAGES:
	@mkdir -p $@

# Compile MO files from PO files
mo: $(MO_FILES) $(RANGE_FILES)
	@echo "MO files compiled for languages: $(LANGUAGES)"

$(MO_FILES_DIR)/%/LC_MESSAGES/$(DOMAIN).mo: $(PO_FILES_DIR)/%.po | $(MO_FILES_DIR)/%/LC_MESSAGES
	@echo "Compiling MO file for $*..."
	msgfmt --check --verbose --output-file="$@" "$<"

$(MO_FILES_DIR)/%/LC_MESSAGES/range: $(MO_FILES_DIR)/%/LC_MESSAGES/$(DOMAIN).mo
	@sed -e "s/./\0\n/g" $(PO_FILES_DIR)/$*.po | sort -u | tr -d '\n' > "$@"

# Create a specific language MO file (e.g., make mo-ja)
mo-%: $(MO_FILES_DIR)/%/LC_MESSAGES/$(DOMAIN).mo
	@echo "MO file for $* created"

# Create a specific language PO file (e.g., make po-ja)
po-%: $(PO_FILES_DIR)/%.po
	@echo "PO file for $* created"

# Update all PO files from POT - using explicit rule
update-po: $(POT_FILE)
	@echo "Updating all PO files..."
	@for lang in $(LANGUAGES); do \
		po_file="$(PO_FILES_DIR)/$$lang.po"; \
		if [ -f "$$po_file" ]; then \
			echo "Updating $$lang..."; \
			msgmerge --update --backup=none "$$po_file" $(POT_FILE); \
		else \
			echo "Creating new PO file for $$lang..."; \
			msginit --no-translator --locale=$$lang --input=$(POT_FILE) --output="$$po_file"; \
		fi; \
	done

# Statistics for translations
stats:
	@echo "Translation statistics:"
	@for lang in $(LANGUAGES); do \
		po_file="$(PO_FILES_DIR)/$$lang.po"; \
		if [ -f "$$po_file" ]; then \
			translated=$$(msgfmt --statistics -o /dev/null "$$po_file" 2>&1 | grep -o '[0-9]*' | head -1); \
			untranslated=$$(msgfmt --statistics -o /dev/null "$$po_file" 2>&1 | grep -o '[0-9]*' | tail -1); \
			total=$$((translated + untranslated)); \
			percent=$$((translated * 100 / total)); \
			printf "  %-8s: %3d%% (%d/%d)\n" "$$lang" "$$percent" "$$translated" "$$total"; \
		else \
			printf "  %-8s: No PO file\n" "$$lang"; \
		fi; \
	done

# Clean generated files
clean:
	@echo "Cleaning generated files..."
	rm -f $(POT_FILE)
	rm -rf $(MO_FILES_DIR)
	@echo "Note: PO files in $(PO_FILES_DIR)/ are preserved."

# Clean everything including PO files
clean-all:
	@echo "Cleaning all generated files..."
	rm -f $(POT_FILE)
	rm -rf $(PO_FILES_DIR)
	rm -rf $(MO_FILES_DIR)

# Help target
help:
	@echo "Translation Makefile Targets:"
	@echo "  all               - Create POT, PO and MO files"
	@echo "  init-languages    - Initialize PO files for all languages"
	@echo "  mo                - Compile MO files from PO files"
	@echo "  update-po         - Update all PO files from POT"
	@echo "  stats             - Show translation statistics"
	@echo "  po-<lang>         - Create PO file for specific language (e.g., po-ja)"
	@echo "  mo-<lang>         - Compile MO file for specific language (e.g., mo-ja)"
	@echo "  clean             - Clean POT and MO files (keep PO files)"
	@echo "  clean-all         - Clean all generated files"
	@echo "  help              - Show this help"
	@echo ""
	@echo "Configured languages: $(LANGUAGES)"
	@echo "Domain: $(DOMAIN)"
	@echo "Bug reports to: $(MSGID_BUGS_ADDRESS)"

# MARK DIRECTORY TARGETS AS PRECIOUS to prevent deletion
.PRECIOUS: $(PO_FILES_DIR)/%.po $(PO_FILES) $(MO_FILES_DIR)/%/LC_MESSAGES

# PHONY targets
.PHONY: all init-languages mo update-po stats clean clean-all help mo-% po-% locales

# Create directories if they don't exist
$(shell mkdir -p $(PO_FILES_DIR))

run: all
	rm -f tl.tar.gz
	tar -C locale -czvf tl.tar.gz .

.DEFAULT_GOAL := run
