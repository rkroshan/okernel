.section .text

.global psci_smcc

psci_smcc:
    hvc #0
    ret