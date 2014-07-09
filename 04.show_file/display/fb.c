
static int FBDeviceInit(void);
static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor);
static int FBCleanScreen(unsigned int dwBackColor);

static int g_iFBFD;
static struct fb_var_screeninfo g_tVar;
static struct fb_fix_screeninfo g_tFix;
static int g_iScreenSize;
static unsigned char * g_pucFBMem;

static int g_iLineWidth;
static int g_iPixelWidth;

/*construct, set and register a structure
* in g_tFBDispOpr
* g represent global variable
* i represent int variable
* t represent a structure variable
*/

static T_DispOpr g_tFBDispOpr = {
	.name = "fb",
	.DeviceInit = FBDeviceInit,
	.ShowPixel = FBShowPixel,
	.CleanScreen = FBCleanScreen,
};

static int FBDeviceInit(void)
{
	g_iFBFD= open(FB_DEVICE_NAME, O_RDWR);
	if(g_iFBFD < 0)
	{
		DBG_PRINTF("can't open %s\n", FB_DEVICE_NAME);
		return -1;
	}

	if(ioctl(g_iFBFD, FBIOGET_VSCREENINFO, &g_tVar))
	{
		DBG_PRINTF("can not get var\n");
		return 01;
	}
	
	if(ioctl(g_iFBFD, FBIOGET_FSCREENINFO,&g_tFix))
	{
		DBG_PRINTF("can't get fix\n");
		return -1;
	}

	g_tFBDispOpr.iXres = g_tVar.xres;
	g_tFBDispOpr.iYres = g_tVar.yres;
	g_tFBDispOpr.iBpp = g_tVar.bits_per_pixel;

	g_iScreenSize = g_tVar.xres * g_tVar.yres * g_tVar.bits_per_pixel /8;
	g_iLineWidth = g_tVar.xres *  g_tVar.bits_per_pixel /8;
	g_iPixelWidth = g_tVar.bits_per_pixel / 8;
	
	g_pucFBMem = (unsigned char *)mmap(NULL, g_iScreenSize, PROT_READ | PROT_WRITE, MAP_SHARED, g_iFBFD, 0);
	if(g_pucFBMem == (unsigned char *)-1)
	{
		DBG_PRINTF("can't mmap\n");
		return -1;
	}
	return 0;
}

static int FBShowPixel(int iPenX, int iPenY, unsigned int dwColor)
{
	unsigned char *p_ucpen_8 = g_pucFBMem + iPenY*g_iLineWidth+iPenX*g_iPixelWidth;
	unsigned short *p_wpen_16;
	unsigned int *p_dwpen_32;

	unsigned int red, green,blue;

	p_wpen_16 = (unsigned short *)p_ucpen_8;
	p_dwpen_32 = (unsigned int *)p_ucpen_8;

	switch (g_tFBDispOpr.iBpp)
	{
		case 8:
		{
			*p_ucpen_8 = dwColor;/*color pallete*/
			break;
			
		}
		case 16:
		{
			/*0xRRGGBB -> 565*/
			red = (dwColor >> 16) & 0xff;
			green = (dwColor >> 8) & 0xff;
			blue = (dwColor >> 0) & 0xff;
			dwColor = ((red >>3) <<11) |((green >>2)<<5) |(blue>>3);
			*p_wpen_16 = dwColor;
			break;
		}
		case 32:
		{
			*p_dwpen_32 = dwColor;
			break;
		}
		default:
		{
			printf("can not support this format: %dbbp\n", g_tFBDispOpr.iBpp);
			return -1;
			break;
		}
	}
	return 0;
}
static int FBCleanScreen(unsigned int dwBackColor)
{
	unsigned char *p_ucpen_8 = g_pucFBMem;
	unsigned short *p_wpen_16;
	unsigned int *p_dwpen_32;

	unsigned int red, green,blue;
	int i;

	p_wpen_16 = (unsigned short *)p_ucpen_8;
	p_dwpen_32 = (unsigned int *)p_ucpen_8;

	
	switch (g_tFBDispOpr.iBpp)
	{
		case 8:
		{
			memset(p_ucpen_8, dwBackColor, g_iScreenSize);/*color pallete*/
			break;
			
		}
		case 16:
		{
			/*0xRRGGBB -> 565*/
			red = (dwBackColor >> 16) & 0xff;
			green = (dwBackColor >> 8) & 0xff;
			blue = (dwBackColor >> 0) & 0xff;
			dwBackColor = ((red >>3) <<11) |((green >>2)<<5) |(blue>>3);

			for(i=0; i<g_iScreenSize; i++)
			{
				*p_wpen_16 = dwBackColor;
				p_wpen_16 ++;
				i+=2;
			}
			break;
		}
		case 32:
		{
			for(i=0; i<g_iScreenSize; i++)
			{
				*p_wpen_32 = dwBackColor;
				p_wpen_32 ++;
				i+=4;
			}
			break;
		}
		default:
		{
			printf("can not support this format: %dbbp\n", g_tFBDispOpr.iBpp);
			return -1;
			break;
		}

	}
	return 0;
}

int FBInit(void)
{
	RegisterDispOpr(&g_tFBDispOpr);
}
