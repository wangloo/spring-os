#include <bitmap.h>
#include <arm64_asid.h>
#include <utils.h>
#include <assert.h>

#define ASID_MAX          4096
#define ASID_FIXED_SHARED 0
#define ASID_FIXED_KERNEL 1
#define ASID_USER_BASE    2

static bitmap_t asid_bitmap[ASID_MAX];
static int asid_max;

int asid_alloc(void)
{
	int asid = 0;

	if (asid_max <= ASID_USER_BASE)
		return 0;

	// spin_lock(&asid_lock);
	asid = bitmap_find_next_0(asid_bitmap, asid_max, ASID_USER_BASE);
	if (asid >= asid_max)
		asid = 0;
	else
		bitmap_set_bit(asid_bitmap, asid);
	// spin_unlock(&asid_lock);

	return asid;
}

void asid_free(int asid)
{
	assert((asid < asid_max) && (asid >= ASID_USER_BASE));
	bitmap_clear_bit(asid_bitmap, asid);
}

void asid_init(void)
{
    asid_max = arch_asid_max();
    asid_max = max(asid_max, ASID_MAX);

    assert(asid_max > ASID_USER_BASE);

    bitmap_set_bits(asid_bitmap, asid_max, ASID_MAX);
    bitmap_set_bit(asid_bitmap, ASID_FIXED_SHARED);
    bitmap_set_bit(asid_bitmap, ASID_FIXED_KERNEL);
}