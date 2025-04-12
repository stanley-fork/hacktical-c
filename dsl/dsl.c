#include <assert.h>
#include <string.h>
#include "dsl.h"

static const uint8_t *push_eval(struct hc_dsl *dsl, const uint8_t *data) {
  struct hc_push_op *op = (void *)data;
  hc_dsl_push(dsl, op->value);
  return data + sizeof(hc_push_op);
}

const struct hc_op hc_push_op = (struct hc_op){
  .code = 0,
  .name = "push",
  .size = sizeof(struct hc_push_op),
  .eval = push_eval
};

void hc_dsl_init(struct hc_dsl *dsl) {
  hc_vector_init(&dsl->code, 1);
  hc_vector_init(&dsl->ops, sizeof(struct hc_op));
  hc_vector_init(&dsl->registers, sizeof(hc_fix));
  hc_vector_init(&dsl->stack, sizeof(hc_fix));

  *(struct hc_op *)hc_vector_push(&dsl->ops) = hc_push_op;
}

void hc_dsl_deinit(struct hc_dsl *dsl) {
  hc_vector_deinit(&dsl->code);
  hc_vector_deinit(&dsl->ops);
  hc_vector_deinit(&dsl->registers);
  hc_vector_deinit(&dsl->stack);
}

void hc_dsl_push(struct hc_dsl *dsl, const hc_fix v) {
  *(hc_fix *)hc_vector_push(&dsl->stack) = v;
}

hc_fix hc_dsl_peek(struct hc_dsl *dsl) {
  return *(hc_fix *)hc_vector_peek(&dsl->stack);
}

hc_fix hc_dsl_pop(struct hc_dsl *dsl) {
  return *(hc_fix *)hc_vector_pop(&dsl->stack);
}

hc_pc hc_dsl_emit(struct hc_dsl *dsl,
		  const struct hc_op *op,
		  const void *data) {
  const hc_pc pc = dsl->code.length;
  *(uint8_t *)hc_vector_push(&dsl->code) = op->code;
  size_t offset = hc_align(dsl->code.end, op->size) - dsl->code.start;
  
  hc_vector_insert(&dsl->code,
		   dsl->code.length,
		   offset + op->size);

  memcpy(dsl->code.start + offset, data, op->size);
  return pc;
}

void hc_dsl_eval(struct hc_dsl *dsl, const hc_pc pc) {
  uint8_t *p = hc_vector_get(&dsl->code, pc);

  while (p < dsl->code.end) {
    struct hc_op *op = hc_vector_get(&dsl->ops, *p);
    p = hc_align(p+1, op->size);
    p = op->eval(dsl, p);
  }
}
