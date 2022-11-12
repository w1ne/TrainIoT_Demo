# IoT train Demo

## Introduction.

A brake test is a safety procedure in rail freight traffic.

The goal of the Brake Test is to evaluate the functionality of the entire brake system. For example, it is carried out to verify that the brakes of a freight train can generate the necessary braking force to stop the train within a certain distance.

Brake Test is performed after each train has been formed when the train does more than 24-hour stops or even when crossing borders.

One of the goals defined by the European Shift to Rail initiative [shift2rail.org] is to deliver automated brake tests and thus reduce the time for train preparation. Automatic brake tests can save time, and money, and make freight transportation more efficient.

In this article, I am budding an architecture and implementation for a prototype IoT device to automatically check the brakes on a freight train demo.

## The problem

Brake tests in freight transport are labor-intensive. The employee is required to check on each axle to whether the brake pads are applied correctly. Verification is required every time cars are disconnected or the train stays still for more than 24 hours. The procedure is done manually: for a 500 m long freight train verification can take up to 40 minutes![https://pjm.co.at/fr/blog/Showroom/automatische-bremsprobe/]

**Safety and economy through automated processes.**
By using an automatic braking system test the staff benefits from avoiding the time-consuming manual check directly at the train, eliminating human error, saving time, and cutting costs for train operators.

## The Brake check procedure

1. Pre-inspection is the first step in a brake check procedure. The goal of pre-inspection is to determine [ARC-WP1-D-DBA-017-01_-_D1.3_%E2%80%93_Automated_Brake_Test.pdf]:
- That all brakes and all parking brakes are released;
- The condition of the brake components (brake blocks are in place and t thickness of the brake blocks is within permitted margins);
- That brakes are turned on (except brakes, which are marked to be non-operational);
- The correct setting of the braking regime and the load charge;
- The correct coupling of the freight cars, and the brake pipe connection.

### Preparation

The driver performs a tightness check.
The reduction of air pressure inside the main brake pipe must not exceed 0,5 bar within one minute (or 0,3 bar for passenger trains).

### Apply brakes

After the preparation check. The driver fills the main brake pipe with 5 bar pressure to release the brakes. Brakes are applied by releasing air pressure from the main brake pipe to 4.2 bar. The driver is visually inspecting each brake and knocks brake blocks with a metal hammer and listening to the sound.

### Release brakes

In the next step, the driver releases the brakes by raising the air pressure in the main brake pipe to 5 bar. The driver walks along the train and checks whether the brakes are released.

The walk driver was conducted to perform a brake check. (Source: Hecht, 2001)

## IoT solution

There are two main topics addressed by the IoT Demonstrator:
1. Automatic detection of the assembled train, using air pressure propagation of the connected cars transmitted by Lora Nodes.
2. Running dummy brake test and reporting results into the IoT dashboard.

Each car of a demonstration train carries out a Lora Mesh Car Node equipped with:
1. Pressure sensor and sensor that reads the position of brake valves;
2. LiPo power distribution unit with a solar panel to gather energy.
Each Car Node communicates data to the driver's unit.

