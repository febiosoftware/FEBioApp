# Getting Started

FEBioApps are small GUI-based applications that can be used to control one or more FEBio models. The app allows you to run FEBio models, collect results, and present them in graphs or 3D views.

## Prerequisites
Before you can use FEBioApps you need to have FEBio Studio installed. 

## The FEBioApp input file

An FEBio App is created from an xml-based input file. The file is divided into two parts: the first part defines the FEBio model, and the second the GUI. 

### The Model Section

The **model** section defines the FEBio model. In most cases, only an id and the file name needs to be defined.

```text
<model id   = "fem"
       file = "ex01.feb"/>
```

### The UI Section
The **ui** section constructs the GUI for the app. The GUI is constructed from sub-elements that can define widgets for labels, buttons, lists, layouts, visualization, and more. 

```text
<ui title="Hello, world!>
   <!-- UI elements here -->
</ui>
```
### The Script Section
The **script** section can be used to collect custom scripts. 

