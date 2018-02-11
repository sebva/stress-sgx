# Stress-SGX: Load and stress your enclaves for fun and profit
# Copyright (C) 2017-2018 Sébastien Vaucher
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
ALL_UNTRUSTED_MK=$(shell find . -name '*sgx_u.mk')
ALL_TRUSTED_MK=$(shell find . -name '*sgx_t.mk')
ALL_STATIC_MK=$(shell find . -name '*sgx_t_static.mk')



.PHONY: all clean run


all clean:
	$(foreach U_MK, $(ALL_UNTRUSTED_MK), $(MAKE) -C $(shell dirname $(U_MK))  -f $(shell basename $(U_MK)) $@;)
	$(foreach T_MK, $(ALL_TRUSTED_MK), $(MAKE) -C $(shell dirname $(T_MK))    -f $(shell basename $(T_MK)) $@;)
	$(foreach U_MK, $(ALL_STATIC_MK), $(MAKE) -C $(shell dirname $(U_MK))  -f $(shell basename $(U_MK)) $@;)

run:
	$(foreach U_MK, $(ALL_UNTRUSTED_MK), $(MAKE) -C $(shell dirname $(U_MK))   -f $(shell basename $(U_MK)) $@;)

