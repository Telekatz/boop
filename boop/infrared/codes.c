/*
    codes.c - structs for accessing the infrared-codetables
    Copyright (C) 2007  Ch. Klippel <ck@mamalala.net>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "codes.h"
#include "ir_lirc.h"

const struct TABLES_S ITT =
{
	4,
	{
		{
			#include "ir_codes/itt/seleco_019"
			"Seleco #19"
		},
		{
			#include "ir_codes/itt/seleco_049"
			"Seleco #49"
		},
		{
			#include "ir_codes/itt/seleco_108"
			"Seleco #108"
		},
		{
			#include "ir_codes/itt/seleco_131"
			"Seleco #131"
		},
	}
};

const struct TABLES_S NRC17 =
{
	2,
	{
		{
			#include "ir_codes/nrc17/dbox2"
			"DBox 2"
		},
		{
			#include "ir_codes/nrc17/grundig_tp720"
			"Grundig TP720"
		}
	}
};


#define RC5_STD(cmd)	0x1000 | cmd
#define RC5X(cmd)		cmd

const struct TABLES_S RC5 =
{
	3,
	{
		{
			#include "ir_codes/rc5/tv"
			"Generic TV"
		},
		{
			#include "ir_codes/rc5/HARMONY_655"
			"Logitec Harmony"
		},
		{
			#include "ir_codes/rc5/technisat_satreceiver_103TS103"
			"SAT(103TS103)"
		}
	}
};

const struct TABLES_I RC6 =
{
	3,
	{
		{
			#include "ir_codes/rc6/tv"
			"RC6 TV"
		},
		{
			#include "ir_codes/rc6/dvd"
			"RC6 DVD"
		},
		{
			#include "ir_codes/rc6/ufs922"
			"UFS 922"
		}
	}
};

const struct TABLES_C RCA =
{
	1,
	{
		{
			#include "ir_codes/rca/xbox"
			"X-Box"
		}
	}
};

const struct TABLES_I RCMM =
{
	2,
	{
		{
			#include "ir_codes/rcmm/galaxis"
			"Galaxis Sat"
		},
		{
			#include "ir_codes/rcmm/ufs910"
			"UFS 910"
		}
	}
};

const struct TABLES_I REC80 =
{
	1,
	{
		{
			#include "ir_codes/rec80/rak_sc957wk"
			"RAK SC-957-WK"
		}
	}
};

const struct TABLES_S RECS80 =
{
	2,
	{
		{
			#include "ir_codes/recs80/seleco_020"
			"seleco#20"
		},
		{
			#include "ir_codes/recs80/seleco_157"
			"Seleco #157"
		}
	}
};

const struct TABLES_S SIRC =
{
	3,
	{
		{
			#include "ir_codes/sirc/rm-pp411"
			"RM-PP411"
		},
		{
			#include "ir_codes/sirc/tv"
			"Generic TV"
		},
		{
			#include "ir_codes/sirc/UFD400"
			"Katrein UFD400"
		}
	}
};

const struct TABLES_C SPACEENC =
{
	2,
	{
		{
			#include "ir_codes/spaceenc/schneider_dtv3"
			"Schneider DTV3"
		},
		{
			#include "ir_codes/spaceenc/seleco_027"
			"Seleco #27"
		}
	}
};

const struct TABLES_S IRRF =
{
	2,
	{
		{
			#include "ir_codes/rf/cc1100_a0"
			"RF 0"
		},
		{
			#include "ir_codes/rf/cc1100_a1"
			"RF 1"
		}
	}
};

const struct TABLES_N RAW =
{
	4,
	{
		{
			"RAW Set 1"
		},
		{
			"RAW Set 2"
		},
		{
			"RAW Set 3"
		},
		{
			"RAW Set 4"
		}
		
	}
};

const struct TABLES_L LIRC =
{
	9,
	{
		{
			#include "ir_codes/lirc/ufs922"
			"UFS 922"
		},
		{
			#include "ir_codes/lirc/str_de545"
			"STR-DE545"
		},
		{
			#include "ir_codes/lirc/ufs910"
			"UFS 910"
		},
		{
			#include "ir_codes/lirc/fs20"
			"FS20"
		},
		{
			#include "ir_codes/lirc/hx2262"
			"HX2262"
		},
		{
			#include "ir_codes/lirc/rev008340"
			"REV 008340"
		},
		{
			#include "ir_codes/lirc/olympus_rm-1"
			"Olympus RM-1"
		},
		{
			#include "ir_codes/lirc/apple_remote1"
			"Apple Remote 1st gen."
		},
		{
			#include "ir_codes/lirc/samsung_ue46b6000"
			"Samsung TV"
		}
	}
};
