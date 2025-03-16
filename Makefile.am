SUBDIRS = src
dist_doc_DATA = README

cppcheck:
	@cppcheck \
		--inline-suppr \
		--enable=warning,style,performance,portability \
		--inconclusive --check-level=exhaustive \
		--template="warning: {file},{line},{severity},{id},{message}" \
		-I . -I src \
		--error-exitcode=1 \
		--std=c++17 \
		--force \
		--verbose .

check-static: cppcheck

