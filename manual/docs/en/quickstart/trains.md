# Quick start: Add and control a train

In Traintastic, you don’t directly control *locomotives*.
Instead, you control **trains**.

A **train** is a collection of vehicles (locomotives and wagons).
- A train may contain **one or more locomotives**.
- The same locomotive or wagon can belong to **different trains**, but it can only be **active in one train at a time**.
- To operate a train, you must **activate it**. Activation only succeeds if none of its locomotives or wagons are already active in another train.

This concept makes it easy to build and re-use different train compositions.

## Step 1: Open the trains and vehicles dialog

1. Make sure you are in **edit mode** (pencil button top right).
2. Open the dialog in one of two ways:
   - From the main menu: **Objects → Trains**
   - Or click the **Train icon** on the toolbar.

The dialog has three tabs:
- **Trains** — define and manage your trains
- **Rail vehicles** — define and manage locomotives and wagons
- **Throttle** — for controlling trains (covered later in the Quick Start series)

> Note: The *Throttle* tab is part of this dialog, but we’ll come back to it after you’ve created and activated a train.

## Step 2: Create a locomotive

TODO: rethink decoder logic, how it works currently is hard to explain.

1. Switch to the **Rail vehicles** tab.
2. Click the **+** button and choose **Locomotive**.
3. Enter the locomotive details:
   - **Name** (e.g. “BR 101” or “My Loco”)
   - **Address** (the DCC/MM/mfx address used by your decoder)
   - Optionally other properties such as function mapping.
4. Close the locomotive dialog.

The locomotive is now in the list of rail vehicles.

## Step 3: Create a train

1. Switch to the **Trains** tab.
2. Click the **+** button to create a new train.
3. Enter a name for the train (e.g. “InterCity” or “Freight Train”).
4. With the new train selected, go to the **Vehicles** tab inside the train dialog.
5. Click **+** and add your locomotive (and optionally wagons).
6. Close the train dialog.

The train is now defined but not yet active.

## Step 4: Activate the train

TODO: rewrite and rethink logic, how it works is hard to explain.

1. Switch to **operate mode** (toggle the pencil button off).
2. Double click on the train in the list, this will open a throttle and automatically activate the train.

If activation succeeds:
- A throttle window opens for controlling speed and direction.
- The train’s status changes to *active*.

If activation fails:
- One of the vehicles may already be active in another train.
  Deactivate the other train first, or adjust your composition.
