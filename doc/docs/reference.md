# Reference

This document describes the various xml elements that can be defined in the FEBioApp input file. For each element, the attributes are listed and a simple example is shown.

## button
Defines a push button. Users can define what happens when the user clicks the button.

* **text** : the text displayed on the button.
* **onClick** : set what happens when the user clicks it. 

```text
<button 
    text    = "Solve"
    onClick = "app.runModel()"/>
```

## graph
This shows a 2D graph. 

```text
<graph	text="stress-strain"
        size="640, 480">
        <data text="sx">
            <x var="Ex" elemId="1"/>
            <y var="sx" elemId="1"/>
        </data>
</graph>
```

## ui
The **ui** element defines the user interface of the app. Child elements define widgets and layout elements.

* **title** : The title shown of the app's window. (optional)

```text
<ui title="My First App"> ... </ui>
```

## hgroup
This item collects child items and displays them in a horizontal row.

* **title** : sets the title (optional)

```text
<hgroup title="params"> ... </hgroup>
```

## input
Defines an input field for editing a model parameter. The actual widget used for editing the field will depend on the associated model parameter. 

* **id**    : The id of the input field. 
* **text**  : The text shown next to the input field. 
* **param** : The model parameter associated with this element.

```text
<input text="G1"
       param="fem.material[0].g1"/>
```

## input_list
This item will create input fields for all the parameters of the specified model component.

* **text** : sets the title of the group
* **params** : defines the model component from which the parameters are extracted. 

```text
<input_list 
    text="Elastic params"
    params="fem.material[0].elastic"/>
```

## label
This item shows a string on the UI. 

```text
<label text="My First Demo"/>
```

## model
Defines the FEBio model that will be used in the app.

* **id** : the id of the model. 

```text
<model id="fem"
       file="ex01.feb"/>
```

## output
Provides a widget that can be used for output or logging. 

* **id** : The id of the widget. 

```text
<output id="log"/>
```

## plot3d
Displays a 3D view of the model.

```text
<plot3d	title="view1"
        size="640, 480"
        bg_color="0.7, 0.9, 0.7">
            <map data="z"/>
</plot3d>
```

## script
The section can be used to collect custom scripts. 

```text
<script>
   ... write script code here ...
</script>
```

## stretch
A stretch element will affect the layout of its parent group. If a stretch element is defined, an empty section is added that can grow when the window is resized. The effect is that when the window is resized, the widgets will stay together.

```text
<stretch/>
```

## tab
Defines a tab inside a **tab_group**. This item can only be defined as a child of a **tab_group**.

* **text** : Set the title of the tab. 

```text
<tab text="Tab1">
    <!-- add widgets here-->
</tab>
```

## tab_group
This item is the parent group for tabs. Each tab should be defined as a child of the tab_group. 

```text
<tab_group>
    <tab text="Graph1">
        <!-- add widgets here-->
    </tab>
    <tab text="Graph2">
        <!-- add widgets here-->
    </tab>
</tab_group>
```

## vgroup
This item collects child items and displays them in a vertical column

* **title** : sets the title (optional)

```text
<vgroup title="params"> ... </vgroup>
```
