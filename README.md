ode-env
=======

With this you can develop computational codes for number of ODE-based models, then set their parameters and initial values, run simulations, and analyze their results. Experiment configurations can be stored for later re-use.

[Add comments here](https://github.com/dimalit/ode-env/issues/1)

## Полиморфизм
На странице ["Концепция проекта"](https://git.io/vHUr2) в разделе 3.2 имеется диаграмма абстрактных классов, от которых наследуют свои классы разработчики подключаемых модулей - моделей, solver'ов, анализаторов. При запуске модельного эксперимента основная программа через эти интерфейсы обращается к конкретным реализациям (например, вызывает перегруженный метод step() в solver'е). Здесь и реализуется полиморфное поведение.
