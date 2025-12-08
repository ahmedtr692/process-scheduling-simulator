# PDF Generation Instructions

The technical documentation is available in Markdown format in `docs/TECHNICAL_DOCUMENTATION.md`.

## Converting to PDF

### Option 1: Using Pandoc (Recommended)

```bash
# Install pandoc
sudo apt-get install pandoc texlive-latex-base texlive-fonts-recommended

# Generate PDF
pandoc docs/TECHNICAL_DOCUMENTATION.md -o docs/TECHNICAL_DOCUMENTATION.pdf \
  --from markdown --pdf-engine=pdflatex \
  --variable geometry:margin=1in \
  --variable fontsize=11pt \
  --toc --toc-depth=3
```

### Option 2: Using Markdown to PDF Online

1. Open https://www.markdowntopdf.com/ or similar service
2. Upload `docs/TECHNICAL_DOCUMENTATION.md`
3. Download the generated PDF

### Option 3: Using Visual Studio Code

1. Install "Markdown PDF" extension
2. Open `docs/TECHNICAL_DOCUMENTATION.md`
3. Right-click and select "Markdown PDF: Export (pdf)"

### Option 4: Using Python markdown2pdf

```bash
# Install markdown2pdf
pip install markdown2pdf

# Generate PDF
markdown2pdf docs/TECHNICAL_DOCUMENTATION.md -o docs/TECHNICAL_DOCUMENTATION.pdf
```

### Option 5: Using wkhtmltopdf

```bash
# Install wkhtmltopdf
sudo apt-get install wkhtmltopdf

# Convert markdown to HTML first (using pandoc or online tool)
pandoc docs/TECHNICAL_DOCUMENTATION.md -o /tmp/tech_doc.html

# Convert HTML to PDF
wkhtmltopdf /tmp/tech_doc.html docs/TECHNICAL_DOCUMENTATION.pdf
```

## Viewing the Documentation

The markdown version is fully readable and contains all the information. GitHub also renders Markdown files beautifully in the repository.
