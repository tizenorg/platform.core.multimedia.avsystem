/*
 * avsystem
 *
 * Copyright (c) 2000 - 2011 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Jonghyuk Choi <jhchoi.choi@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <stdio.h>
#include <alloca.h>
#include <string.h>
#include "../include/avsys-audio.h"
#include "../include/avsys-audio-logical-volume.h"

#define VERSION_NUM	0001


void usage(char *name)
{
	printf("Usage: %s [FILE]...\n\n", name);
	printf("Version %04d\n", VERSION_NUM);
	printf("\n");
	return;
}

int main(int argc, char* argv[])
{
	int i = 0;
	int j = 0;
	int k = 0;
	int err = 0;
	char filepath[256]={0,};
	FILE *fp = NULL;

	if(argc > 2)
	{
		usage(argv[0]);
		return 0;
	}
	else if(argc == 2)
	{
		strncpy(filepath, argv[1], sizeof(filepath)-1);
	}
	else if(argc == 1)
	{
		strncpy(filepath, VOLUME_FILE_PATH, sizeof(filepath)-1);
		fprintf(stderr,"Use default file path %s\n", filepath);
	}

	fp = fopen(filepath, "w");
	if(!fp)
	{
		fprintf(stderr,"Can not open file %s\n",filepath);
		return -1;
	}

	for(i=AVSYS_AUDIO_LVOL_GAIN_TYPE_0; i<AVSYS_AUDIO_LVOL_GAIN_TYPE_MAX; i++)
	{
		for(j=AVSYS_AUDIO_LVOL_DEV_TYPE_SPK; j<=AVSYS_AUDIO_LVOL_DEV_TYPE_BTHEADSET;j++)
		{
			int lv = 0;
			int rv = 0;
			char *str_dev[] = { "SPK", "HEADSET", "BTHEADSET" };

			for(k=0;k<LVOLUME_MAX_MULTIMEDIA;k++)
			{
				err = avsys_audio_get_volume_table(i, AVSYS_AUDIO_LVOL_DEV_TYPE_SPK, k, &lv, &rv);
				if(err)
					goto FAIL;

				fprintf(fp,"%1d:%s:%d:%d:%d\n",i,str_dev[j],k,lv,rv);
			}
		}
	}

	if(fp != NULL)
	{
		fclose(fp);
	}
	printf("Success\n");
	return 0;

FAIL:
	if(fp != NULL)
	{
		fclose(fp);
	}
	unlink(filepath);
	fprintf(stderr,"Can not dump volume table\n");
	return -1;
}
