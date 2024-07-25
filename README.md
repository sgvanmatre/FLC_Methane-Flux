# Open-source Methane Flux Instrument

Methane, a potent greenhouse gas, has significantly increased in the Earth’s atmosphere since the Industrial Revolution. A considerable portion of methane emissions stems from ground-to-atmosphere methane flux, originating from sources such as landfills, geologic seeps, and agricultural activities. These emissions exhibit spatial complexity and temporal variability, posing challenges in accurate characterization and mitigation.

Quantifying methane flux from these diverse sources is hindered by the lack of affordable measurement tools suitable for widespread deployment. There is a critical need for an instrument capable of measuring flux rates spanning 0.1 to 1000 g CH4 m-2 d-1 at landfills. The ideal instrument must operate autonomously with a remote power supply, and be compact and lightweight enough for easy transport by two individuals. It should provide flux measurements at hourly intervals and support cellular data transmission.

Through a collaborative effort between Fort Lewis College and CU Boulder's air quality laboratories, a low-cost, dynamic flux chamber was developed, calibrated, and validated for measuring methane flux within the range of 0-150 g/m²-day. Methane concentrations in the sensor chamber are monitored using four low-cost metal oxide sensors: a BME 680, along with Figaro TGS sensors (2600, 2602, and 2611).

This repository includes the PCB design and the microcontroller code for the system. 

## Table of Contents

- [Usage](#usage)
- [Contributing](#contributing)
- [Credits](#credits)
- [License](#license)
- [Updating Firmware](#updating-firmware)


## Usage

We encourage everyone to use or adapt this code for their own projects. Below are some examples and instructions to help you get started.

### Tools Used

- **Eagle PCB Software:** We used Eagle PCB software for designing and testing our printed circuit boards.
- **Arduino:** Our code is compatible with the Arduino platform and was developed using the Arduino IDE and initialized on an Arduino MKR NB 1500 board. We did have to manually update the firmware. Instruction are at the bottom of this file.

### Getting Started

1. **Clone the Repository:**
   - Clone the repository to your local machine using the following command:
     ```sh
     git clone https://github.com/your-username/your-repo-name.git
     ```

2. **Open in Eagle PCB Software:**
   - Open the Eagle PCB files from the repository in Eagle PCB software to view and modify the PCB designs.

3. **Load the Code in Arduino IDE:**
   - Open the Arduino IDE.
   - Load the Arduino code from the repository into the Arduino IDE.

4. **Upload to Your Arduino Board:**
   - Connect your Arduino board to your computer.
   - Select the correct board and port from the Arduino IDE.
   - Click the "Upload" button to upload the code to your Arduino board.

## Contributing

We welcome contributions from anyone who is interested! Whether it's fixing a bug, adding a new feature, improving documentation, or any other enhancements, your help is greatly appreciated.

### How to Contribute

1. **Fork the Repository:**
   - Click the "Fork" button at the top right of this repository page to create a copy of this repository in your GitHub account.

2. **Clone Your Fork:**
   - Clone your forked repository to your local machine using the following command:
     ```sh
     git clone https://github.com/your-username/your-repo-name.git
     ```

3. **Create a New Branch:**
   - Create a new branch to work on your changes:
     ```sh
     git checkout -b your-branch-name
     ```

4. **Make Your Changes:**
   - Implement your changes in the new branch.

5. **Commit Your Changes:**
   - Commit your changes with a clear and concise commit message:
     ```sh
     git add .
     git commit -m "Description of your changes"
     ```

6. **Push Your Changes:**
   - Push your changes to your forked repository:
     ```sh
     git push origin your-branch-name
     ```

7. **Submit a Pull Request:**
   - Go to the original repository on GitHub and click the "New Pull Request" button.
   - Select your branch from your forked repository and compare it with the base repository.
   - Submit the pull request for review.

### Guidelines

- Ensure your code follows the project's style guidelines.
- Write clear, concise commit messages.
- Update documentation as necessary.
- Test your changes thoroughly before submitting a pull request.

Thank you for your interest in contributing! Together, we can make this project even better.


## Credits

Contributors

- **Principal Contributor:** [Seth VanMatre](https://github.com/sgvanmatre)
- **Summer Contributions:** [Sonie Taylor Schermer](https://github.com/tschermer02)
- **Additional Assistance:** [Max Krauss](https://github.com/maxtkrauss)

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Firmware Update Instructions

The latest SARA-R410M-02B firmware (version -04) is now available publicly on the product page:

[Product Page - SARA-R4 Series](https://www.u-blox.com/en/product/sara-r4-series?legacy=Current#Documentation-&-resources)

A direct download to the firmware files is available here:

[Direct Download - Firmware Files](https://content.u-blox.com/sites/default/files/2023-06/SARA-R410M-02B-04-P1-L0000000512A0221-000K00.zip?_ga=2.26117621.998416448.1694721819-824184379.1689262389)

The recommended method is to update via the native USB port of the SARA-R4 module using our Windows tool "EasyFlash".

To download the correct EasyFlash tool, refer to the link in the PCN to the most up-to-date/recommended version in the section "Tools."

A link to the latest (version -04) PCN is available here:

[PCN - Version -04](https://content.u-blox.com/sites/default/files/documents/SARA-R410M-02B-04_PCN_UBX-22005059.pdf)

For updating the firmware, an application note is available here:

[Application Note - Firmware Update](https://www.u-blox.com/sites/default/files/SARA-R4-FW-Update_AppNote_UBX-17049154.pdf)

### Additional Resources for 3rd Party Boards

- **Arduino MKR NB 1500 USB test points:**
  [Forum Discussion](https://forum.arduino.cc/t/firmware-upgrade-for-ublox-sara-r410m-02b-on-the-mkr-nb-1500-2/699292)

- **Particle Boron:**
  [Community Guide](https://community.particle.io/t/how-to-load-ublox-firmware-upgrade-for-sara-r410-lte-on-the-boron/51769)

- **Sparkfun LTE M1 Shield enable USB:**
  [Hookup Guide](https://learn.sparkfun.com/tutorials/lte-cat-m1nb-iot-shield-hookup-guide?_gl=1*pm7rgo*_ga*NDUwNDc5ODYxLjE2OTMyNDMzMjk.*_ga_T369JS7J9N*MTY5NDc5ODg0My4yLjAuMTY5NDc5ODg0My42MC4wLjA.&_ga=2.145880316.975271267.1694798844-450479861.1693243329#USB-interface)

