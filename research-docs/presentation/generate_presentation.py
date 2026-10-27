from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor

prs = Presentation()
prs.slide_width = Inches(13.333)
prs.slide_height = Inches(7.5)

NAVY = RGBColor(18, 35, 62)
BLUE = RGBColor(34, 104, 180)
WHITE = RGBColor(255, 255, 255)
GRAY = RGBColor(85, 96, 110)

def add_slide(title, points):
    slide = prs.slides.add_slide(
        prs.slide_layouts[6]
    )

    background = slide.background.fill
    background.solid()
    background.fore_color.rgb = WHITE

    title_box = slide.shapes.add_textbox(
        Inches(0.7), Inches(0.4),
        Inches(12), Inches(0.8)
    )

    tf = title_box.text_frame
    tf.text = title

    for p in tf.paragraphs:
        p.font.size = Pt(30)
        p.font.bold = True
        p.font.color.rgb = NAVY

    body = slide.shapes.add_textbox(
        Inches(0.9), Inches(1.5),
        Inches(11.5), Inches(5.4)
    )

    tf = body.text_frame
    tf.word_wrap = True

    for index, point in enumerate(points):
        p = tf.paragraphs[0] if index == 0 else tf.add_paragraph()
        p.text = point
        p.font.size = Pt(22)
        p.font.color.rgb = GRAY
        p.space_after = Pt(18)

    return slide

output = "research-docs/presentation/Adaptive_EWMA_MCSRW_Presentation.pptx"

add_slide("Adaptive EWMA-Based Spinning for MCSRW Locks", [
    "Operating Systems Research Project",
    "Adaptive synchronization for reader-writer locks",
    "Based on the OptiQL research repository"
])

add_slide("Introduction", [
    "Reader-writer locks allow concurrent readers and exclusive writers.",
    "Waiting threads may repeatedly spin while acquiring locks.",
    "Spinning behavior can affect throughput under contention."
])

add_slide("Research Problem", [
    "Fixed spinning policies do not respond to observed waiting behavior.",
    "Adaptive policies introduce additional computation.",
    "Research question: Can EWMA-based spinning outperform Fixed64?"
])

add_slide("Existing MCSRW Lock", [
    "MCSRW provides synchronization for readers and writers.",
    "Threads coordinate access through the existing locking protocol.",
    "Our research modifies waiting behavior, not the underlying protocol."
])

add_slide("Proposed Adaptive EWMA Approach", [
    "Each thread maintains its own adaptive spinning state.",
    "Observed spin counts update an EWMA estimate.",
    "The estimate determines the spinning threshold."
])

add_slide("EWMA Algorithm", [
    "Initial smoothed wait = 64; initial threshold = 64",
    "EWMA = (3 * previous EWMA + spin count) / 4",
    "EWMA < 32: threshold 256",
    "EWMA < 128: threshold 64",
    "Otherwise: threshold 0"
])

add_slide("System Architecture", [
    "Reader / Writer Thread",
    "MCSRW Lock Waiting Loop",
    "Observe Spin Count",
    "Update Thread-Local EWMA",
    "Select Threshold: 0, 64 or 256",
    "Apply Selected Spinning Policy"
])

add_slide("Implementation and Correctness", [
    "Implemented adaptive spinning in MCSRW.h.",
    "Created a compile-time Fixed64 comparison mode.",
    "Adaptive EWMA: 40,000 expected and actual updates; 0 violations.",
    "Fixed64: 40,000 expected and actual updates; 0 violations.",
    "Both passed the conducted correctness tests."
])

add_slide("Experimental Methodology", [
    "OptiQL latch microbenchmark framework.",
    "4 threads; 5-second benchmark trials.",
    "80% version reads; 20% acquire-release operations.",
    "Lock counts: 128, 5 and 1.",
    "Five paired trials per main comparison.",
    "Environment: Ubuntu under WSL."
])

add_slide("Throughput Comparison", [
    "Experiment 28 (5 locks): EWMA +2.12%",
    "Experiment 32 (128 locks): EWMA -2.89%",
    "Experiment 33 (5 locks): EWMA -2.72%",
    "Experiment 34 (1 lock): EWMA -1.76%",
    "All reported paired 95% confidence intervals included zero."
])

add_slide("Findings and Limitations", [
    "Threshold 64 dominated adaptive selections.",
    "Spin-count saturation occurred in fewer than 1% of recorded waits.",
    "No statistically reliable throughput advantage was demonstrated.",
    "Limitations: WSL variability, limited workloads and five paired trials."
])

add_slide("Conclusion and Future Work", [
    "Successfully implemented and evaluated adaptive EWMA spinning.",
    "Both configurations passed the conducted correctness tests.",
    "Adaptive spinning did not demonstrate a reliable speedup over Fixed64.",
    "Future work: native Linux, more workloads and alternative thresholds."
])

prs.save(output)
print(f"Presentation generated: {output}")
print(f"Total slides: {len(prs.slides)}")
