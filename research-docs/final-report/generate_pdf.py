from pathlib import Path
from xml.sax.saxutils import escape
from reportlab.platypus import (
    SimpleDocTemplate, Paragraph, Spacer, Preformatted,
    Table, TableStyle, KeepTogether
)
from reportlab.lib import colors
from reportlab.lib.styles import getSampleStyleSheet, ParagraphStyle
from reportlab.lib.enums import TA_CENTER
from reportlab.lib.units import inch

folder = Path(__file__).resolve().parent
source = folder / "FINAL_RESEARCH_REPORT.md"
output = folder / "FINAL_RESEARCH_REPORT.pdf"

styles = getSampleStyleSheet()
styles.add(ParagraphStyle(
    name="ReportTitle", parent=styles["Title"],
    fontSize=18, leading=24, alignment=TA_CENTER,
    spaceAfter=22
))
styles.add(ParagraphStyle(
    name="ReportBody", parent=styles["BodyText"],
    fontSize=10, leading=15, spaceAfter=8
))
styles.add(ParagraphStyle(
    name="ReportCode", fontName="Courier",
    fontSize=8, leading=11
))

doc = SimpleDocTemplate(
    str(output), pagesize=(595.28, 841.89),
    rightMargin=48, leftMargin=48,
    topMargin=50, bottomMargin=50
)

story = []
lines = source.read_text(encoding="utf-8").splitlines()
i = 0

while i < len(lines):
    line = lines[i].strip()

    if not line:
        story.append(Spacer(1, 5))
        i += 1
        continue

    if line.startswith("|"):
        rows = []
        while i < len(lines) and lines[i].strip().startswith("|"):
            cells = [c.strip() for c in lines[i].strip().strip("|").split("|")]
            if not all(set(c) <= set("-: ") for c in cells):
                rows.append([
                    Paragraph(escape(c), styles["ReportCode"])
                    for c in cells
                ])
            i += 1

        if rows:
            width = (doc.width / len(rows[0]))
            table = Table(rows, colWidths=[width] * len(rows[0]),
                          repeatRows=1, hAlign="LEFT")
            table.setStyle(TableStyle([
                ("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#E8EEF5")),
                ("GRID", (0, 0), (-1, -1), 0.3, colors.grey),
                ("VALIGN", (0, 0), (-1, -1), "TOP"),
                ("BOTTOMPADDING", (0, 0), (-1, -1), 6),
                ("TOPPADDING", (0, 0), (-1, -1), 6),
            ]))
            story.append(table)
        continue

    if line.startswith("# "):
        story.append(Paragraph(escape(line[2:]), styles["ReportTitle"]))
    elif line.startswith("## "):
        story.append(Paragraph(escape(line[3:]), styles["Heading1"]))
    elif line.startswith("### "):
        story.append(Paragraph(escape(line[4:]), styles["Heading2"]))
    elif lines[i].startswith("    "):
        block = []
        while i < len(lines) and lines[i].startswith("    "):
            block.append(lines[i][4:])
            i += 1
        story.append(Preformatted("\n".join(block), styles["ReportCode"]))
        continue
    else:
        story.append(Paragraph(escape(line), styles["ReportBody"]))

    i += 1

doc.build(story)
print(f"PDF generated: {output}")
