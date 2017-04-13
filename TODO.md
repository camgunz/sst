# To Do

- I think the tokenizer is skipping spaces when counting columns; check

- TemplateCache:
  - Stores parsed templates so `include` blocks don't need to re-parse
    templates (would be nice to detect cycles at runtime too).
  - Some threading concerns however.

