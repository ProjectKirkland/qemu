#define DPE_REG_MAX 3
#define DPE_MAILBOX_WORDS 256
#define DPE_MAILBOX_SIZE (DPE_MAILBOX_WORDS * sizeof(uint32_t))

REG32(DPE_CMD_SIZE, 0x00)
REG32(DPE_CMD_EXECUTE, 0x04)
REG32(DPE_CMD_STATUS, 0x08)

#define DPE_ADDR_BASE 0xfed30000

#define DPE(x) ((DPEState *)x)

typedef struct DPEState {
    DeviceState parent_obj;

    uint32_t regs[DPE_REG_MAX];
    MemoryRegion mmio;
    MemoryRegion mailbox;
} DPEState;

enum dpe_execute {
    DPE_EXECUTE_RUN = BIT(0),
    DPE_EXECUTE_IDLE = BIT(1),
}

enum dpe_status {
    DPE_STATUS_SUCCESS = BIT(0),
    DPE_STATUS_BUSY = BIT(1),
    DPE_STATUS_ERROR = BIT(2),
};

static uint64_t dpe_mmio_read(void *opaque, hwaddr addr,
                                  unsigned size)
{
    DPEState *s = DPE(opaque);


}

static void dpe_mmio_write(void *opaque, hwaddr addr,
                               uint64_t val, unsigned size)
{
    DPEState *s = DPE(opaque);
    switch (addr) {
        case A_DPE_CMD_SIZE:
            s->regs[R_DPE_CMD_SIZE] = val;
            break;
        case A_DPE_CMD_EXECUTE:
            if (val == DPE_EXECUTE_RUN) {
                s->regs[R_DPE_CMD_STATUS] |= DPE_STATUS_BUSY;
            }
            s->regs[R_DPE_CMD_EXECUTE] = val;

            void *mem = memory_region_get_ram_ptr(&s->mailbox);

            size_t in_size = MIN(DPE_MAILBOX_SIZE, s->regs[R_DPE_CMD_SIZE];
            dpe_backend_deliver_request(s->dpebe, mem, in_size, mem, DPE_MAILBOX_SIZE);
            break;
    }
}

static void dpe_request_completed(DPEIf *di, int ret) {
    DPEState *s = DPE(ti);
    s->regs[R_DPE_CMD_EXECUTE] = DPE_EXECUTE_IDLE;
    if (reg != 0) {
        s->regs[R_DPE_CMD_STATUS] = DPE_STATUS_ERROR;
    } else {
        s->regs[R_DPE_CMD_STATUS] = DPE_STATUS_SUCCESS;
    }

    memory_region_set_dirty(&s->mailbox, 0, DPE_MAILBOX_SIZE);
}

static const MemoryRegionOps dep_memory_ops = {
    .read = dpe_mmio_read,
    .write = dpe_mmio_write,
    .endianness = DEVICE_LITTLE_ENDIAN,
    .valid = {
        .min_access_size = 1,
        .max_access_size = 4,
    }
}

static const VMStateDescription vmstate_dpe = {
    .name = "dpe",
    .fields = (VMStateField[]) {
        VMSTATE_UINT32_ARRAY(regs, DPEState, DPE_REG_MAX),
        VMSTATE_END_OF_LIST(),
    }
};

static Proprty dpe_properties[] = {
    DEFINE_PROP_END_OF_LIST(),
};

static void dpe_realize(DeviceState *dev, Error **errp) {
    DPEState *s = DPE(dev);

    memory_region_init_io(&s->mmio, OBJECT(s), &dpe_memory_ops, s, "dpe-mmio", sizeof(s->regs));
    memory_region_init_ram(&s->mailbox, OBJECT(s), "dpe-mailbox", sizeof(cmd_buf), errp);

    memory_region_add_subregion(get_system_memory(), DPE_ADDR_BASE, &s->mmio);
    memory_region_add_subregion(get_system_memory(), DPE_ADDR_BASE + sizeof(s->regs), &s->mailbox);

}

static void dpe_class_init(ObjectClass *oc, void *data) {
    DeviceClass *dc = DEVICE_CLASS(oc);

    dc->realize = dpe_realize;
    device_class_set_props(dc, dpe_properties);
    dc->vmsd  = &vmstate_dpe;
    dc->user_creatable = true;

    set_bit(DEVICE_CATEGORY_MISC, dc->categories);
}

static const TypeInfo dpe_info = {
    .name = TYPE_DPE,
    .parent = TYPE_DEVICE,
    .instance_size = 0,
    .class_init = dpe_class_init,
};

static void dpe_register(void)
{
    type_register_static();
}

type_init(dpe_register)
