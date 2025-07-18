# Scripting

This document describes how to use scripting to make UI elements talk to each other.

The scripting language used in the app's input file is similar to JavaScript. 

## app
The app object has the following member functions.

* **runModel**: Starts the FEBio model
* **stopModel**: Stops the FEBio model that is running.
* **quit**: Exits the app.

## ui
The ui object is used to query ui elements. To find a particular ui element, use the **getElementById** function. This function requires one argument, namely the id of the ui element. 

```text
w = ui.getElementById('startButton');
```
