#
# Copyright (C) 2020 Linux Studio Plugins Project <https://lsp-plug.in/>
#           (C) 2020 Vladimir Sadovnikov <sadko4u@gmail.com>
#
# This file is part of lsp-runtime-lib
#
# lsp-runtime-lib is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# any later version.
#
# lsp-runime-lib is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with lsp-runtime-lib.  If not, see <https://www.gnu.org/licenses/>.
#

# Variables that describe dependencies
LSP_COMMON_LIB_VERSION     := 1.0.7
LSP_COMMON_LIB_NAME        := lsp-common-lib
LSP_COMMON_LIB_TYPE        := src
LSP_COMMON_LIB_URL         := https://github.com/sadko4u/$(LSP_COMMON_LIB_NAME).git

LSP_LLTL_LIB_VERSION       := 0.5.2
LSP_LLTL_LIB_NAME          := lsp-lltl-lib
LSP_LLTL_LIB_TYPE          := src
LSP_LLTL_LIB_URL           := https://github.com/sadko4u/$(LSP_LLTL_LIB_NAME).git

LSP_TEST_FW_VERSION        := 1.0.5
LSP_TEST_FW_NAME           := lsp-test-fw
LSP_TEST_FW_TYPE           := src
LSP_TEST_FW_URL            := https://github.com/sadko4u/$(LSP_TEST_FW_NAME).git

LIBSNDFILE_VERSION         := system
LIBSNDFILE_NAME            := sndfile
LIBSNDFILE_TYPE            := pkg

ifeq ($(PLATFORM),Windows)
  STDLIB_VERSION             := system
  STDLIB_TYPE                := opt
  STDLIB_LDFLAGS             := -lpthread -lshlwapi -lwinmm -lmsacm32
else ifeq ($(PLATFORM),BSD)
  STDLIB_VERSION             := system
  STDLIB_TYPE                := opt
  STDLIB_LDFLAGS             := -lpthread -ldl -liconv
else
  STDLIB_VERSION             := system
  STDLIB_TYPE                := opt
  STDLIB_LDFLAGS             := -lpthread -ldl
endif

ifeq ($(PLATFORM),Windows)
  TEST_STDLIB_VERSION        := system
  TEST_STDLIB_TYPE           := opt
  TEST_STDLIB_LDFLAGS        := 
else
  TEST_STDLIB_VERSION        := system
  TEST_STDLIB_TYPE           := opt
  TEST_STDLIB_LDFLAGS        := 
endif
