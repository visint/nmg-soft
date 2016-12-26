#include <stdio.h>
#include <string.h>
#include <unistd.h>
int main()
{
  FILE *fp;
	char buf[80];
	char scrbuf[80];
	unsigned long total, used, mfree, shared, buffers, cached;
        int scr_width;
	/* read memory info */
	fp = fopen("/proc/meminfo","r");

	/*
	 * Old kernels (such as 2.4.x) had a nice summary of memory info that
	 * we could parse, however this is gone entirely in 2.6. Try parsing
	 * the old way first, and if that fails, parse each field manually.
	 *
	 * First, we read in the first line. Old kernels will have bogus
	 * strings we don't care about, whereas new kernels will start right
	 * out with MemTotal:
	 *                              -- PFM.
	 */
 if (fscanf(fp, "MemTotal: %lu %s\n", &total, buf) != 2) {
		fgets(buf, sizeof(buf), fp);    /* skip first line */

		fscanf(fp, "Mem: %lu %lu %lu %lu %lu %lu",
			&total, &used, &mfree, &shared, &buffers, &cached);
		/* convert to kilobytes */
		used /= 1024;
		mfree /= 1024;
		shared /= 1024;
		buffers /= 1024;
		cached /= 1024;
		total /= 1024;
	} else {
		/*
		 * Revert to manual parsing, which incidentally already has the
		 * sizes in kilobytes. This should be safe for both 2.4 and
		 * 2.6.
		 */
		fscanf(fp, "MemFree: %lu %s\n", &mfree, buf);

		/*
		 * MemShared: is no longer present in 2.6. Report this as 0,
		 * to maintain consistent behavior with normal procps.
		 */
		if (fscanf(fp, "MemShared: %lu %s\n", &shared, buf) != 2)
			shared = 0;

		fscanf(fp, "Buffers: %lu %s\n", &buffers, buf);
		fscanf(fp, "Cached: %lu %s\n", &cached, buf);

		used = total - mfree;
	}
	fclose(fp);
  /* output memory info */
	if (scr_width > (int)sizeof(scrbuf))
		scr_width = sizeof(scrbuf);
	snprintf(scrbuf, scr_width,
		"Mem: %luK used, %luK free, %luK shrd, %luK buff, %luK cached",
		used, mfree, shared, buffers, cached);
      printf("used=%d mfree=%d usage:%.1f\n",used, mfree,used*100.0/(used+mfree));
}
