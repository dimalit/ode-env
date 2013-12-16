ode-env
=======

With this you can develop computational codes for number of ODE-based models, then set their parameters and initial values, run simulations, and analyze their results. Experiment configurations can be stored for later re-use.

[Add comments here](https://github.com/dimalit/ode-env/issues/1)

## Полиморфизм
На странице ["Концепция проекта"](/dimalit/ode-env/wiki/%D0%9A%D0%BE%D0%BD%D1%86%D0%B5%D0%BF%D1%86%D0%B8%D1%8F-%D0%BF%D1%80%D0%BE%D0%B5%D0%BA%D1%82%D0%B0) в разделе 3.2 имеется диаграмма абстрактных классов, от которых наследуют свои классы разработчики подключаемых модулей - моделей, solver'ов, анализаторов. При запуске модельного эксперимента основная программа через эти интерфейсы обращается к кокретным реализациям (например, вызывает перегруженный метод step() в solver'е).
